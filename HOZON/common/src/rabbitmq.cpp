#include "rabbitmq.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <iostream>
#include <cstring>

namespace hozon {

static bool checkAmqpError(amqp_rpc_reply_t reply, const char* context) {
    switch (reply.reply_type) {
        case AMQP_RESPONSE_NORMAL:
            return true;

        case AMQP_RESPONSE_NONE:
            std::cerr << context << ": missing RPC reply type" << std::endl;
            return false;

        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            std::cerr << context << ": " << amqp_error_string2(reply.library_error) << std::endl;
            return false;

        case AMQP_RESPONSE_SERVER_EXCEPTION:
            if (reply.reply.id == AMQP_CHANNEL_CLOSE_METHOD) {
                amqp_channel_close_t* m = (amqp_channel_close_t*)reply.reply.decoded;
                std::cerr << context << ": server channel error "
                         << m->reply_code << ": "
                         << std::string((char*)m->reply_text.bytes, m->reply_text.len)
                         << std::endl;
            } else if (reply.reply.id == AMQP_CONNECTION_CLOSE_METHOD) {
                amqp_connection_close_t* m = (amqp_connection_close_t*)reply.reply.decoded;
                std::cerr << context << ": server connection error "
                         << m->reply_code << ": "
                         << std::string((char*)m->reply_text.bytes, m->reply_text.len)
                         << std::endl;
            } else {
                std::cerr << context << ": unknown server error, method id " << reply.reply.id << std::endl;
            }
            return false;

        default:
            return false;
    }
}

class RabbitMQPublisher::Impl {
public:
    amqp_connection_state_t conn_;
    amqp_socket_t* socket_;
    std::string host_;
    int port_;
    std::string user_;
    std::string password_;
    bool connected_;

    Impl(const std::string& host, int port, const std::string& user, const std::string& password)
        : conn_(nullptr), socket_(nullptr), host_(host), port_(port),
          user_(user), password_(password), connected_(false) {
        connect();
    }

    ~Impl() {
        disconnect();
    }

    void connect() {
        conn_ = amqp_new_connection();
        socket_ = amqp_tcp_socket_new(conn_);

        if (!socket_) {
            std::cerr << "RabbitMQ: Failed to create TCP socket" << std::endl;
            return;
        }

        int status = amqp_socket_open(socket_, host_.c_str(), port_);
        if (status != AMQP_STATUS_OK) {
            std::cerr << "RabbitMQ: Failed to open socket to " << host_ << ":" << port_ << std::endl;
            return;
        }

        amqp_rpc_reply_t reply = amqp_login(conn_, "/", 0, 131072, 0,
                                            AMQP_SASL_METHOD_PLAIN,
                                            user_.c_str(), password_.c_str());
        if (!checkAmqpError(reply, "Login")) {
            return;
        }

        amqp_channel_open(conn_, 1);
        reply = amqp_get_rpc_reply(conn_);
        if (!checkAmqpError(reply, "Opening channel")) {
            return;
        }

        connected_ = true;
        std::cout << "RabbitMQ Publisher connected to " << host_ << ":" << port_ << std::endl;
    }

    void disconnect() {
        if (conn_) {
            amqp_channel_close(conn_, 1, AMQP_REPLY_SUCCESS);
            amqp_connection_close(conn_, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn_);
            conn_ = nullptr;
        }
        connected_ = false;
    }

    void reconnect() {
        disconnect();
        connect();
    }

    void declareQueue(const std::string& queueName) {
        if (!connected_) return;

        // passive=0, durable=1 (персистентная), exclusive=0, auto_delete=0
        amqp_queue_declare(conn_, 1,
                          amqp_cstring_bytes(queueName.c_str()),
                          0, 1, 0, 0,
                          amqp_empty_table);
        amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn_);
        checkAmqpError(reply, "Declaring queue");
    }

    bool publish(const std::string& queueName, const std::string& message) {
        if (!connected_) {
            reconnect();
            if (!connected_) return false;
        }

        amqp_basic_properties_t props;
        props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
        props.content_type = amqp_cstring_bytes("application/json");
        props.delivery_mode = 2;  // персистентное сообщение

        int status = amqp_basic_publish(conn_, 1,
                                        amqp_cstring_bytes(""),
                                        amqp_cstring_bytes(queueName.c_str()),
                                        0, 0, &props,
                                        amqp_cstring_bytes(message.c_str()));

        if (status != AMQP_STATUS_OK) {
            std::cerr << "RabbitMQ: Failed to publish message" << std::endl;
            return false;
        }

        return true;
    }
};

RabbitMQPublisher::RabbitMQPublisher(const std::string& host, int port,
                                     const std::string& user, const std::string& password)
    : impl_(std::make_unique<Impl>(host, port, user, password)) {}

RabbitMQPublisher::~RabbitMQPublisher() = default;

void RabbitMQPublisher::declareQueue(const std::string& queueName) {
    impl_->declareQueue(queueName);
}

bool RabbitMQPublisher::publish(const std::string& queueName, const std::string& message) {
    return impl_->publish(queueName, message);
}

bool RabbitMQPublisher::isConnected() const {
    return impl_->connected_;
}

void RabbitMQPublisher::reconnect() {
    impl_->reconnect();
}

class RabbitMQConsumer::Impl {
public:
    amqp_connection_state_t conn_;
    amqp_socket_t* socket_;
    std::string host_;
    int port_;
    std::string user_;
    std::string password_;
    std::atomic<bool> running_;
    std::thread consumerThread_;

    Impl(const std::string& host, int port, const std::string& user, const std::string& password)
        : conn_(nullptr), socket_(nullptr), host_(host), port_(port),
          user_(user), password_(password), running_(false) {
        connect();
    }

    ~Impl() {
        stop();
        disconnect();
    }

    void connect() {
        conn_ = amqp_new_connection();
        socket_ = amqp_tcp_socket_new(conn_);

        if (!socket_) {
            std::cerr << "RabbitMQ Consumer: Failed to create TCP socket" << std::endl;
            return;
        }

        int status = amqp_socket_open(socket_, host_.c_str(), port_);
        if (status != AMQP_STATUS_OK) {
            std::cerr << "RabbitMQ Consumer: Failed to open socket" << std::endl;
            return;
        }

        amqp_rpc_reply_t reply = amqp_login(conn_, "/", 0, 131072, 0,
                                            AMQP_SASL_METHOD_PLAIN,
                                            user_.c_str(), password_.c_str());
        if (!checkAmqpError(reply, "Consumer Login")) {
            return;
        }

        amqp_channel_open(conn_, 1);
        reply = amqp_get_rpc_reply(conn_);
        if (!checkAmqpError(reply, "Consumer Opening channel")) {
            return;
        }

        std::cout << "RabbitMQ Consumer connected to " << host_ << ":" << port_ << std::endl;
    }

    void disconnect() {
        if (conn_) {
            amqp_channel_close(conn_, 1, AMQP_REPLY_SUCCESS);
            amqp_connection_close(conn_, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(conn_);
            conn_ = nullptr;
        }
    }

    void declareQueue(const std::string& queueName) {
        if (!conn_) return;

        amqp_queue_declare(conn_, 1,
                          amqp_cstring_bytes(queueName.c_str()),
                          0, 1, 0, 0,
                          amqp_empty_table);
        amqp_get_rpc_reply(conn_);
    }

    void consume(const std::string& queueName, MessageCallback callback) {
        if (running_) return;

        running_ = true;

        consumerThread_ = std::thread([this, queueName, callback]() {
            amqp_basic_qos(conn_, 1, 0, 1, 0);  // prefetch=1 для равномерного распределения

            // no_local=0, no_ack=0 (ручное подтверждение), exclusive=0
            amqp_basic_consume(conn_, 1,
                              amqp_cstring_bytes(queueName.c_str()),
                              amqp_empty_bytes,
                              0, 0, 0,
                              amqp_empty_table);

            amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn_);
            if (!checkAmqpError(reply, "Consuming")) {
                running_ = false;
                return;
            }

            std::cout << "Started consuming from queue: " << queueName << std::endl;

            while (running_) {
                amqp_envelope_t envelope;
                amqp_maybe_release_buffers(conn_);

                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                reply = amqp_consume_message(conn_, &envelope, &timeout, 0);

                if (reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION) {
                    if (reply.library_error == AMQP_STATUS_TIMEOUT) {
                        continue;
                    }
                    std::cerr << "Consumer error: " << amqp_error_string2(reply.library_error) << std::endl;
                    continue;
                }

                if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
                    continue;
                }

                std::string message(
                    static_cast<char*>(envelope.message.body.bytes),
                    envelope.message.body.len
                );

                bool processed = false;
                try {
                    processed = callback(message);
                } catch (const std::exception& e) {
                    std::cerr << "Error processing message: " << e.what() << std::endl;
                }

                if (processed) {
                    amqp_basic_ack(conn_, 1, envelope.delivery_tag, 0);
                } else {
                    amqp_basic_nack(conn_, 1, envelope.delivery_tag, 0, 1);  // вернуть в очередь
                }

                amqp_destroy_envelope(&envelope);
            }
        });
    }

    void stop() {
        running_ = false;
        if (consumerThread_.joinable()) {
            consumerThread_.join();
        }
    }
};

RabbitMQConsumer::RabbitMQConsumer(const std::string& host, int port,
                                   const std::string& user, const std::string& password)
    : impl_(std::make_unique<Impl>(host, port, user, password)) {}

RabbitMQConsumer::~RabbitMQConsumer() = default;

void RabbitMQConsumer::declareQueue(const std::string& queueName) {
    impl_->declareQueue(queueName);
}

void RabbitMQConsumer::consume(const std::string& queueName, MessageCallback callback) {
    impl_->consume(queueName, callback);
}

void RabbitMQConsumer::stop() {
    impl_->stop();
}

bool RabbitMQConsumer::isRunning() const {
    return impl_->running_;
}

} // namespace hozon
