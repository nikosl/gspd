#include <string>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <iostream>
#include <vector>

int main() {
    zmq::context_t ctx;
    zmq::socket_t sock(ctx, zmq::socket_type::push);
    sock.bind("inproc://test");
    sock.send(zmq::str_buffer("Hello, world"), zmq::send_flags::dontwait);
    std::vector<std::string> vec;
    vec.emplace_back("Hello");
    vec.emplace_back("MessagePack");

    // serialize it into simple buffer.
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, vec);

    // deserialize it.
    msgpack::object_handle oh =
            msgpack::unpack(sbuf.data(), sbuf.size());

    // print the deserialized object.
    msgpack::object obj = oh.get();
    std::cout << obj << std::endl;  //=> ["Hello", "MessagePack"]
}
