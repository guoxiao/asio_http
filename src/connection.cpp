//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <utility>
#include <vector>
#include <iostream>
#include "request_handler.hpp"

namespace http {
namespace server {

connection::connection(asio::io_service& io_service, request_handler& handler)
  : socket_(io_service),
    request_handler_(handler),
    strand_(io_service),
    keep_alive_(false)
{
}

asio::ip::tcp::socket& connection::socket() {
  return socket_;
}

void connection::start()
{
  do_read();
}

void connection::stop()
{
  socket_.close();
}

void connection::do_read()
{
  auto self(shared_from_this());
  socket_.async_read_some(asio::buffer(buffer_),
      strand_.wrap([this, self](std::error_code ec, std::size_t bytes_transferred)
      {
        if (!ec)
        {
          request_parser::result_type result;
          std::tie(result, std::ignore) = request_parser_.parse(
              request_, buffer_.data(), buffer_.data() + bytes_transferred);

          if (result == request_parser::good)
          {
            auto it = std::find_if(request_.headers.begin(), request_.headers.end(), [](header item) {
                return item.name == "Connection";
            });
            if (it != request_.headers.end()) {
              std::transform(it->value.begin(), it->value.end(), it->value.begin(), ::tolower);
              keep_alive_ = it->value == "keep-alive";
            }
            request_.keep_alive = keep_alive_;

            request_handler_.handle_request(request_, reply_);
            do_write();
          }
          else if (result == request_parser::bad)
          {
            reply_ = reply(reply::bad_request);
            do_write();
          }
          else
          {
            do_read();
          }
        }
      }));
}

void connection::do_write()
{
  auto self(shared_from_this());
  asio::async_write(socket_, reply_.to_buffers(),
                    strand_.wrap([this, self](std::error_code ec, std::size_t)
      {
        std::cout << "error_code" << ec.message() << std::endl;
        if (!ec)
        {
          if (keep_alive_) {
            request_parser_.reset();
            do_read();
          } else {
            // Initiate graceful connection closure.
            asio::error_code ignored_ec;
            socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
                ignored_ec);
          }
        } else {
          asio::error_code ignored_ec;
          socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }
      }));
}

} // namespace server
} // namespace http
