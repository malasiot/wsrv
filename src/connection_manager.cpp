//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <ws/detail/connection_manager.hpp>
#include <ws/detail/connection.hpp>

namespace ws {

ConnectionManager::ConnectionManager()
{
}

void ConnectionManager::start(ConnectionPtr c)
{
    boost::unique_lock<boost::mutex> lock(mutex_) ;

    connections_.insert(c);
    c->start();
}

void ConnectionManager::stop(ConnectionPtr c)
{
    boost::unique_lock<boost::mutex> lock(mutex_) ;
    if ( connections_.count(c) )
          connections_.erase(c);
    c->stop();
}

void ConnectionManager::stop_all()
{
    boost::unique_lock<boost::mutex> lock(mutex_) ;
    for (auto c: connections_)
        c->stop();
    connections_.clear();
}

} // namespace ws
