# Web Server in C

I am attempting at writing a functional web server in C for educational purposes.

## To do:

[x] - Set up a server simple "Hello World" server

[] - Implement routing: Hash table &/or binary search tree

[] - Serve web pages

[] - Handle Error 404

[] - Implement cache

[] - Concurrency (IPC)

## Components of the server

A response generated from chatgpt when prompted with the question "What is needed in a web server program written in C?"

1. Socket programming: The web server program must be able to create and manage sockets to communicate with clients over the network.

2. HTTP request handling: The web server program must be able to receive and parse HTTP requests from clients, and generate appropriate HTTP responses.

3. File handling: The web server program must be able to read files from the file system and send them as a response to client requests.

4. Multi-threading or multi-processing: To handle multiple client requests simultaneously, the web server program may need to support multi-threading or multi-processing.

5. Security: The web server program must be designed to prevent security vulnerabilities such as buffer overflows, injection attacks, and other types of attacks that could compromise the server or client data.

6. Configuration: The web server program should have a configuration file that can be used to specify the server settings, such as the port number, document root directory, and other options.

7. Logging: The web server program should be able to log server activity, such as client requests, errors, and other events.

8. Performance optimization: The web server program may need to be optimized for performance to handle a large number of client requests efficiently.

## Sources

* [Beej's guide to network programming](https://beej.us/guide/bgnet/html/)
* [Medium post on creating a web server in c](https://aleksazatezalo.medium.com/creating-a-web-server-in-c-55ad33ff51b3)
* [Short video on creating a web server in c](https://www.youtube.com/watch?v=cEH_ipqHbUw&ab_channel=ImranRahman)
