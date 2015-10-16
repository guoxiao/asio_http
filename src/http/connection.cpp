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
#include <string>
#include <iostream>
#include <http_parser.h>

namespace http {
namespace server {

connection::connection(asio::io_service& io_service, request_handler& handler)
  : socket_(io_service),
    request_handler_(handler),
    strand_(io_service)
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
      asio::wrap(strand_, [this, self](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          http_parser_settings settings;
          http_parser_settings_init(&settings);

          settings.on_url = [](http_parser *parser, const char* url, size_t len) {
            request* req = static_cast<request*>(parser->data);
            req->uri = std::string(url, len);
            req->method = std::string(http_method_str(static_cast<http_method>(parser->method)));
            return 0;
          };

          std::unique_ptr<http_parser> parser = std::unique_ptr<http_parser>(new http_parser());
          http_parser_init(parser.get(), HTTP_REQUEST);
          parser->data = &request_;

          size_t nparsed = http_parser_execute(parser.get(), &settings, buffer_.data(), bytes_transferred);

          if (nparsed == bytes_transferred && !parser->upgrade) {
            request_handler_.handle_request(request_, reply_);
            do_write();
          } else if (parser->upgrade) {
            reply_ = reply::stock_reply(reply::not_implemented);
            do_write();
          } else if (parser->http_errno) {
            std::cerr << "Http error: " << http_errno_name(static_cast<http_errno>(parser->http_errno)) << std::endl;
            std::cerr << "Http error: " << http_errno_description(static_cast<http_errno>(parser->http_errno)) << std::endl;
            reply_ = reply::stock_reply(reply::bad_request);
            do_write();
          } else if (!http_body_is_final(parser.get())) {
            do_read();
          }
        }
      }));
}

void connection::do_write()
{
  auto self(shared_from_this());
  asio::async_write(socket_, reply_.to_buffers(),
                    asio::wrap(strand_, [this, self](std::error_code ec, std::size_t)
      {
        if (!ec)
        {
          // Initiate graceful connection closure.
          asio::error_code ignored_ec;
          socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }
      }));
}

} // namespace server
} // namespace http
