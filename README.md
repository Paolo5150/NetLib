# Network Library

A flexible, high-performance network library built on top of [Asio](https://think-async.com/Asio/), offering support for both **TCP** and **UDP** communication. This library provides inheritable server/client classes and supports templated message handling for customizable communication protocols. It also includes examples of integration with [FlatBuffers](https://google.github.io/flatbuffers/) for efficient, schema-based data serialization.

## Features
- **TCP and UDP Support**: Provides server and client classes for both connection-oriented (TCP) and connectionless (UDP) communication.
- **Templated Message Handling**: Flexible and type-safe messaging through templates, allowing you to define your own message structures.
- **Asynchronous I/O**: Powered by Asio for non-blocking network operations, ensuring high performance and scalability.
- **FlatBuffers Support**: Integrates seamlessly with FlatBuffers to handle efficient, schema-driven message serialization.
- **Packet Reassembly**: Automatically assembles and processes fragmented UDP packets, handling out-of-order delivery.

