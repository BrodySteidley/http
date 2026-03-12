
#ifndef BRODYS_HTTP_SERVER_INCLUDED
#define BRODYS_HTTP_SERVER_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define HTTP_HEADER_OK \
       	"HTTP/1.1 200 OK\r\n" \
	"Content-Type: %s\r\n" \
	"Content-Length: %ld\r\n\r\n"

#define HTTP_HEADER_REDIRECT \
	"HTTP/1.1 301 Moved Permanently\r\n" \
	"Location: %s\r\n" \
	"Content-Length: 0\r\n\r\n"

#define HTTP_HEADER_ERROR \
	"HTTP/1.1 404 Not Found\r\n" \
	"Content-Length: 27\r\n\r\n" \
	"<h1>404 page not found</h1>"

typedef enum HttpRequestMethod
{
	HTTP_REQUEST_INVALID,

	/* defined in RFC 9110 */
	HTTP_REQUEST_GET,
	HTTP_REQUEST_HEAD,
	HTTP_REQUEST_POST,
	HTTP_REQUEST_PUT,
	HTTP_REQUEST_DELETE,
	HTTP_REQUEST_CONNECT,
	HTTP_REQUEST_OPTIONS,
	HTTP_REQUEST_TRACE,
	
	/* defined in RFC 5789 */
	HTTP_REQUEST_PATCH
} HttpRequestMethod;

unsigned httpserver_init(unsigned portNumber);
/* Initializes the server; httpserver_kill() should be called afterward.
 * returns 0 on success, > 0 on failure */

void httpserver_kill();
/* terminates the server, should be called after httpserver_init() */

unsigned httpserver_receive(char* resultBuffer, unsigned resultBufferLen);
/* reads the latest message from socket to resultBuffer
 * resultBuffer will be zeroed on socket failure, otherwise filled with the request string
 * returns 0 on success, >0 on failure
 * a server should typically terminate on receive failure */

HttpRequestMethod httpserver_getHttpRequestMethod(char* requestBuffer);
/* requestBuffer is assumed to be at least 9 characters long, filled by the result of httpserver_receive()
 * detects the HttpRequestMethod from a client request string */

bool httpserver_sendString(char* response);
/* send a null-terminated string to the client,
 * returns 1 on send success, 0 on send failure */

unsigned httpserver_sendProcessedFile(const char* filename, char* contentType,
	void (*preprocess)(char** filecontent, unsigned long* filelength,
		const char* filename, const char* contentType));
/* Reads and sends a file to the server, returns 0 if a file was sent, > 0 otherwise
 * filename and contentType are expected to be null terminated strings
 * If contentType is NULL, will guess the content type based on the file extension;
 * it will default to a text file if the file extension is not recognized.
 * If preproccess() is not null, the contents and length of the file are passed to it before the data is sent.  */

#define httpserver_sendFile(filename, contentType) \
	httpserver_sendProcessedFile(filename, contentType, NULL)

bool httpserver_sendRedirect(const char* redirect);
/* send redirect. returns 1 on send success, 0 on send failure */

#define httpserver_sendError() \
	httpserver_sendString(HTTP_HEADER_ERROR)

bool httpserver_getClientIpv4(unsigned char addr[4]);
/* returns 1 on success, 0 on failure; will fail if server or socket not initialized.
 * httpserver_receive must be called to initialize the socket
 * returns the ip address in each field char of addr,
 * i.e. [0].[1].[2].[3];  127.0.0.1 => addr[0] = 127, addr[1] = 0, addr[2] = 0, addr[3] = 1 */

bool httpserver_getClientIpv4String(char buffer[17]);
/* returns 1 on success, 0 on failure; will fail if server or socket not initialized.
 * httpserver_receive must be called to initialize the socket
 * returns address as a null terminated string in human readable format, x.x.x.x
 * 16 is the maximum length of an address, 17 for null terminator */

#endif /* BRODYS_HTTP_SERVER_INCLUDED */

