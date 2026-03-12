

## HTTP Server in C

Here is a small http library in C on Unix; it is effectively just a wrapper for \<netinet/in.h\> and \<sys/socket.h\>

The fileserver/ directory contains an example file server implementation using the library; it treats http requests as file paths and returns the file if found.

To compile and run it:
```
cd fileserver/
cc main.c ../http.c -o fileserver
./fileserver
```
You would then visit http://\<your device's local ip\>:8080 in a browser to see the result.

