#include <iostream>
#include "http/server.hpp"
#include "http/request.hpp"
#include "http/reply.hpp"

int main(int argc, char *argv[]) {
  // Initialise the server.
  http::server::server s("127.0.0.1", "8080", 1);

  s.add_handler("/hello", [](const http::server::request &req, http::server::reply &rep) {
    using namespace http::server;
    rep.content = "hello world!";
    rep.status = reply::ok;
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = std::to_string(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "application/json";
  });

  // Run the server until stopped.
  s.run();
  return 0;
}
