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

* TODO 1. Modify your program to parse the request. You can ignore all of the headers for now. For now, you're only responding to validly formatted GET requests. Send the dummy message back for any validly formatted GET requests. If the request is improperly formatted, respond 400. For any other valid requests apart from GET requests, respond with 501.

* TODO 2. Modify your program to take another command line argument for the "root" directory. Make a directory somewhere, and put a dummy HTML file called index.html and another dummy HTML file called whatever you want. Add a dummy image file as well. When your server starts up, verify that the folder exists and that your program has permissions to view the contents. Modify your program to parse the path from valid GET requests. Upon parsing the path, check the root folder to see if a file matches that filename. If so, respond 200, read the file and write the file to the client socket. If the path is "/" (ie. the root directory) serve index.html. If the requested file does not exist, respond 404 not found. Make sure your solution works for text files as well as binaries (ie. images).

* TODO 3. Add a couple of folders to your root folder, and add dummy html files (and dummy index.html files) to them. Add a few levels of nested folders. Modify your program to improve the path-parsing logic to handle folders, and handle responses appropriately.

* TODO 4. Modify the permissions on a few dummy folders and files to make their read permissions forbidden to your server program. Implement the 403 response appropriately. Scrutinize the URI standard, and modify your path-parsing to strip out (and handle) any troublesome characters. Modify your path-parsing to handle (ignore) query strings and fragments. (? and #). Modify your path-parsing to ensure it does not allow malicious folder navigation using "..".

## Sources

* [Beej's guide to network programming](https://beej.us/guide/bgnet/html/)
* [Medium post on creating a web server in c](https://aleksazatezalo.medium.com/creating-a-web-server-in-c-55ad33ff51b3)
* [Short video on creating a web server in c](https://www.youtube.com/watch?v=cEH_ipqHbUw&ab_channel=ImranRahman)
* [Checklist of what to include in my web server](https://old.reddit.com/r/C_Programming/comments/kbfa6t/building_a_http_server_in_c/gfh8kid/)
