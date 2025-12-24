#ifndef HOZON_DATABASE_H
#define HOZON_DATABASE_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include <functional>
#include <pqxx/pqxx>

namespace hozon {

using DbRow = std::map<std::string, std::string>;
using DbResult = std::vector<DbRow>;

class Database {
public:
    Database(const std::string& host,
             int port,
             const std::string& dbname,
             const std::string& user,
             const std::string& password);

    ~Database();

    void execute(const std::string& query);
    DbResult query(const std::string& query);

    template<typename... Args>
    DbResult queryParams(const std::string& query, Args&&... args);

    template<typename... Args>
    int executeParams(const std::string& query, Args&&... args);

    void beginTransaction();
    void commit();
    void rollback();

    bool executeTransaction(std::function<bool()> callback);

    pqxx::connection& getConnection();
    bool isConnected() const;
    void reconnect();

private:
    std::unique_ptr<pqxx::connection> conn_;
    std::unique_ptr<pqxx::work> transaction_;
    std::string connectionString_;

    void ensureConnected();
    DbResult resultToDbResult(const pqxx::result& result);
};

template<typename... Args>
DbResult Database::queryParams(const std::string& query, Args&&... args) {
    ensureConnected();
    pqxx::work txn(*conn_);
    pqxx::result result = txn.exec_params(query, std::forward<Args>(args)...);
    txn.commit();
    return resultToDbResult(result);
}

template<typename... Args>
int Database::executeParams(const std::string& query, Args&&... args) {
    ensureConnected();
    pqxx::work txn(*conn_);
    pqxx::result result = txn.exec_params(query, std::forward<Args>(args)...);
    txn.commit();
    return result.affected_rows();
}

} // namespace hozon

#endif // HOZON_DATABASE_H
