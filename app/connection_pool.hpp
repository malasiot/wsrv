#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <xdb/connection.hpp>

class ConnectionPool {
public:
    // Define a custom deleter that returns the connection to the pool instead of destroying it
    struct ConnectionDeleter {
        ConnectionPool* pool_ptr_;
        void operator()(xdb::Connection* session) const {
            if ( pool_ptr_ && session ) {
                pool_ptr_->releaseConnection(session);
            }
        }
    };

    using ConnectionPtr = std::unique_ptr<xdb::Connection, ConnectionDeleter>;

    ConnectionPool(const std::string& url, size_t pool_size) : connection_url_(url), max_size_(pool_size) {
        for (size_t i = 0; i < max_size_ ; ++i) {
            try {
                auto session = std::make_unique<xdb::Connection>(connection_url_);
                pool_.push(session.release());
            } catch (const std::exception& e) {
                std::cerr << "Failed to initialize pool connection: " << e.what() << "\n";
                throw;
            }
        }
    }

    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    ~ConnectionPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            delete pool_.front();
            pool_.pop();
        }
    }

    // Thread-safe method to borrow a connection. Blocks if pool is empty.
    ConnectionPtr acquireConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until a connection becomes available
        cv_.wait(lock, [this]() { return !pool_.empty(); });

        xdb::Connection* session = pool_.front();
        pool_.pop();

        // Wrap raw pointer in unique_ptr with our custom deleter
        return ConnectionPtr(session, ConnectionDeleter{this});
    }

private:
    // Called automatically when ConnectionPtr goes out of scope
    void releaseConnection(xdb::Connection* session) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(session);
        cv_.notify_one(); // Wake up one waiting thread
    }

    std::string connection_url_ ;
    size_t max_size_ ;
    std::queue<xdb::Connection *> pool_;
    std::mutex mutex_;
    std::condition_variable cv_;
};