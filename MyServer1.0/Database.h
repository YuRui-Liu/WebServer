#pragma once

#include "Logger.h"
#include <sqlite3.h>
#include <string>
#include <mutex>
#include <stdexcept>


class Database {
private:
    sqlite3* db;
    std::mutex dbMutex; // 互斥锁，用于同步对数据库的访问

public:
    // 构造函数，用于打开数据库并创建用户表
    Database(const std::string& db_path) {
        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }
        // 创建用户表的SQL语句
        const char* sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT);";
        char* errmsg;
        //sqlite3_exec 是 SQLite C API 中的一个函数，用于执行一条或一组 SQL 命令，并处理其结果
        //这里如果执行 sqlite3_exec 函数并尝试执行 SQL 命令时发生了任何错误，
        //那么该条件将会成立，程序可能接下来会进行错误处理，比如打印或显示由 errmsg 指向的错误消息。
        if (sqlite3_exec(db, sql, 0, 0, &errmsg) != SQLITE_OK) {
            throw std::runtime_error("Failed to create table: " + std::string(errmsg));
        }
    }

    // 析构函数，用于关闭数据库连接
    ~Database() {
        sqlite3_close(db);
    }

    // 用户注册函数
    bool registerUser(const std::string& username, const std::string& password) {
        std::lock_guard<std::mutex> guard(dbMutex); // 锁定互斥锁
        std::string sql = "INSERT INTO users (username, password) VALUES (?, ?);";
        sqlite3_stmt* stmt;

        // 准备SQL语句

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            LOG_INFO("Failed to prepare registration SQL for user: %s" , username.c_str()); // 记录日志
            return false;
        }

        // 绑定参数
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        // 执行SQL语句
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            LOG_INFO("Registration failed for user: %s " , username.c_str()); // 记录日志
            sqlite3_finalize(stmt);
            return false;
        }

        // 完成操作，关闭语句
        sqlite3_finalize(stmt);
        LOG_INFO("User registered: %s with password: %s" , username.c_str(), password.c_str()); // 记录日志
        return true;
    }

    // 用户登录函数
    bool loginUser(const std::string& username, const std::string& password) {
        std::lock_guard<std::mutex> guard(dbMutex); // 锁定互斥锁
        std::string sql = "SELECT password FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;

        // 准备SQL语句

        // int sqlite3_prepare_v2(
        //     sqlite3 *db,            // 指向已打开的SQLite数据库连接
        //     const char *zSql,        // 包含SQL命令的以空字符终止的字符串
        //     int nByte,               // SQL命令的字节数，或-1表示使用整个字符串直到遇到'\0'
        //     sqlite3_stmt **ppStmt,   // 输出参数，将指向新创建的预编译语句对象
        //     const char **pzTail      // 可选输出参数，指向未被编译的部分（通常在处理多条SQL时有用）
        //     );
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            LOG_INFO("Failed to prepare login SQL for user: %s" , username.c_str()); // 记录日志
            return false;
        }

        // 绑定参数函数原型
        // int sqlite3_bind_text(sqlite3_stmt* stmt, 
        //               int index, 
        //               const char* value, 
        //               int n, 
        //               void(*destroy)(void*) /* 或者使用 SQLITE_TRANSIENT */);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

        // 执行SQL语句
        //功能：执行预编译的 SQL 语句（prepared statement）。它会推进到下一个结果行或者直到整个查询完成。
        //对于 SELECT 查询，每调用一次 sqlite3_step，就会获取下一行数据；
        //对于 INSERT, UPDATE, DELETE 等非查询操作，则会在操作成功完成后返回。
        //返回值：在处理 SELECT 查询时，如果还有更多的数据行可读取，将返回 SQLITE_ROW；
        //当查询完全执行完毕且没有错误时，返回 SQLITE_DONE。
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            LOG_INFO("User not found: %s" , username.c_str()); // 记录日志
            sqlite3_finalize(stmt);
            return false;
        }

        // 获取存储的密码并转换为std::string
        //功能：该函数用于获取SQLite查询结果集中指定列的数据字节数，
        //不包括结束符（对于文本数据）。如果查询结果当前行的指定列是一个BLOB或者字符串类型，则返回实际数据的长度。
        //用法：在成功执行了SQL查询且调用 sqlite3_step 后，你可以循环遍历结果集中的每一行，
        //然后对每一列调用 sqlite3_column_bytes 来获取该列数据的大小。
        const char* stored_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string password_str(stored_password, sqlite3_column_bytes(stmt, 0));

        // 检查密码是否匹配
        //sqlite3_finalize：
        //功能：这个函数是用来释放预编译的SQL语句（也称为准备好的语句或预编译语句柄）所占用的资源。
        //当使用 sqlite3_prepare_v2 或相关函数成功预编译了一个SQL查询后，该查询会关联一个内部表示，并在内存中保留一些状态信息，
        //以便多次执行相同的查询而无需每次都解析和编译。
        //用法：在完成所有查询执行并不再需要预编译的SQL语句时，应该调用 sqlite3_finalize 函数，
        //传入预编译语句句柄作为参数。这样可以释放与该句柄相关的资源，防止内存泄漏。
        sqlite3_finalize(stmt);
        if (stored_password == nullptr || password != password_str) {
            LOG_INFO("Login failed for user: %s password:%s stored password is %s" ,username.c_str() ,password.c_str(), password_str.c_str()); // 记录日志
            return false;
        }

        // 登录成功，记录日志
        LOG_INFO("User logged in: %s" , username.c_str());
        return true;
    }
};
