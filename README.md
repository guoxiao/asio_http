# asio_http
A simple multithread async http server based on asio

## Usage

```cpp
using namespace http::server;
server s("127.0.0.1", "8080", 1);

s.add_handler("/hello", [](const request &req, reply &rep) {
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
```

See `example/main.cpp` for a complete example

## Build

```bash
mkdir build && cd build
cmake .. -DBUILD_EXAMPLE -DCMAKE_BUILD_TYPE=Release
```

## License

MIT License

Based on the examples from asio licensed by Boost License.
