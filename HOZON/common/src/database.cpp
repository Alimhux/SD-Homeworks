#include "database.h"
#include <sstream>
#include <iostream>

namespace hozon {

Database::Database(const std::string& host,
                   int port,
                   const std::string& dbname,
                   const std::string& user,
                   const std::string& password) {
    std::ostringstream ss;
    ss << "host=" << host
       << " port=" << port
       << " dbname=" << dbname
       << " user=" << user
       << " password=" << password;

    connectionString_ = ss.str();

    try {
        conn_ = std::make_unique<pqxx::connection>(connectionString_);
        std::cout << "Connected to database: " << dbname << "@" << host << ":" << port << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        throw;
    }
}

Database::~Database() {
    if (transaction_) {
        try {
            transaction_->abort();
        } catch (...) {}
    }
}

void Database::ensureConnected() {
    if (!conn_ || !conn_->is_open()) {
        reconnect();
    }
}

void Database::reconnect() {
    try {
        conn_ = std::make_unique<pqxx::connection>(connectionString_);
    } catch (const std::exception& e) {
        std::cerr << "Reconnection failed: " << e.what() << std::endl;
        throw;
    }
}

bool Database::isConnected() const {
    return conn_ && conn_->is_open();
}

pqxx::connection& Database::getConnection() {
    ensureConnected();
    return *conn_;
}

void Database::execute(const std::string& query) {
    ensureConnected();
    pqxx::work txn(*conn_);
    txn.exec(query);
    txn.commit();
}

DbResult Database::query(const std::string& query) {
    ensureConnected();
    pqxx::work txn(*conn_);
    pqxx::result result = txn.exec(query);
    txn.commit();
    return resultToDbResult(result);
}

DbResult Database::resultToDbResult(const pqxx::result& result) {
    DbResult dbResult;

    for (const auto& row : result) {
        DbRow dbRow;
        for (int i = 0; i < static_cast<int>(row.size()); ++i) {
            std::string colName = result.column_name(static_cast<pqxx::row::size_type>(i));
            dbRow[colName] = row[i].is_null() ? "" : row[i].as<std::string>();
        }
        dbResult.push_back(dbRow);
    }

    return dbResult;
}

void Database::beginTransaction() {
    ensureConnected();
    if (transaction_) {
        throw std::runtime_error("Transaction already in progress");
    }
    transaction_ = std::make_unique<pqxx::work>(*conn_);
}

void Database::commit() {
    if (!transaction_) {
        throw std::runtime_error("No transaction in progress");
    }
    transaction_->commit();
    transaction_.reset();
}

void Database::rollback() {
    if (!transaction_) {
        throw std::runtime_error("No transaction in progress");
    }
    transaction_->abort();
    transaction_.reset();
}

bool Database::executeTransaction(std::function<bool()> callback) {
    ensureConnected();
    pqxx::work txn(*conn_);

    try {
        bool success = callback();
        if (success) {
            txn.commit();
        } else {
            txn.abort();
        }
        return success;
    } catch (...) {
        txn.abort();
        throw;
    }
}

} // namespace hozon
