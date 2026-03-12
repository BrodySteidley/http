
#include "http.h"


static int file_descriptor;

static struct sockaddr_in server_address;
static socklen_t server_addrlen;
static bool server_initialized;

static int client_socket;
static struct sockaddr_in client_address;
static socklen_t client_addrlen; 
static bool client_socket_open;

unsigned httpserver_init(unsigned portNumber)
{
	if (server_initialized)
		return 0;

	if ((file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Socket Descriptor Generation Failed!\n");
		return 1;
	}
    
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(portNumber);

	server_addrlen = (socklen_t) sizeof(server_address);
    

	if (bind(file_descriptor, (struct sockaddr*) &(server_address), server_addrlen) < 0)
	{
		fprintf(stderr, "Binding Address to Socket Description Failed!\n");
		return 2;
	}

	if (listen(file_descriptor, 3) < 0)
	{
		return 3;
	}

	client_socket = client_socket_open;

	server_initialized = 1;
	return 0;
}

void httpserver_kill(void)
{
	if (!server_initialized)
		return;

	if (client_socket_open)
	{
		close(client_socket);
		client_socket_open = 0;
	}

	close(file_descriptor);
	server_initialized = 0;
}

static void catchSIGINT(int signal)
{
#ifdef _VERBOSE
	printf("\nhttpserver: SIGINT received\n");
#endif
	httpserver_kill();
}

unsigned httpserver_receive(char* resultBuffer, unsigned resultBufferLen)
{
	if (!resultBuffer)
		return 1;
	
	for (int i = 0; i < resultBufferLen; ++i)
		resultBuffer[i] = '\0';

	if (!server_initialized)
		return 2;
	
	if (client_socket_open)
	{
		close(client_socket); /* close client_socket if left open */
		client_socket_open = 0;
	}
				      
#ifdef _VERBOSE
	printf("httpserver: Listening..\n");
#endif

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catchSIGINT;
	sa.sa_flags = SA_RESTART; /* interrupt accept() and read() system calls when SIGINT occurs */
	
	struct sigaction *oldAction = NULL;

	if (sigaction(SIGINT, &sa, oldAction) == -1)
	{
#ifdef _VERBOSE
		printf("httpserver: unable to capture SIGINT in case of interrupt.\n");
#endif
		return 3;
	}

	client_addrlen = sizeof(client_address);
	client_socket = accept(file_descriptor, (struct sockaddr*) &(client_address), &client_addrlen);
	
	if (client_socket < 0)
		return 4;
		
	if (read(client_socket, resultBuffer, resultBufferLen - 1) == -1)
	{
		for (int i = 0; i < resultBufferLen; ++i)
			resultBuffer[i] = '\0';
		close(client_socket);
		return 5;	
	}
	

#ifdef _VERBOSE
	printf("- - - - - - - -\nRecieved:\n%s\n", resultBuffer);
#endif

	/* return old action */
	if (!oldAction)
		signal(SIGINT, SIG_DFL);
	else if (sigaction(SIGINT, oldAction, NULL) == -1)
	{
#ifdef _VERBOSE
		printf("httpserver: unable to return SIGINT to previous action for some reason.\n");
#endif
	}

	client_socket_open = 1;
	/* the socket is left open, _send() functions can be used */
	return 0;
}


HttpRequestMethod httpserver_getHttpRequestMethod(char* requestBuffer)
{
	
	if (!strncmp(requestBuffer, "GET ", 4))
		return HTTP_REQUEST_GET;
	
	if (!strncmp(requestBuffer, "HEAD ", 5))
		return HTTP_REQUEST_HEAD;
	
	if (!strncmp(requestBuffer, "POST ", 5))
		return HTTP_REQUEST_POST;
	
	if (!strncmp(requestBuffer, "PUT ", 4))
		return HTTP_REQUEST_PUT;
	
	if (!strncmp(requestBuffer, "DELETE ", 7))
		return HTTP_REQUEST_DELETE;
	
	if (!strncmp(requestBuffer, "CONNECT ", 8))
		return HTTP_REQUEST_CONNECT;
	
	if (!strncmp(requestBuffer, "OPTIONS ", 8))
		return HTTP_REQUEST_OPTIONS;
	
	if (!strncmp(requestBuffer, "TRACE ", 6))
		return HTTP_REQUEST_TRACE;
			

	if (!strncmp(requestBuffer, "PATCH ", 6))
		return HTTP_REQUEST_PATCH;
	
	return HTTP_REQUEST_INVALID;
}


bool httpserver_sendString(char* response)
{
	if (!server_initialized || !client_socket_open)
		return 0;

#ifdef _VERBOSE
	printf("Returning:\n\n%s\n- - - - - - - -\n\n", response);
#endif
    	send(client_socket, response, strlen(response), 0);
	return 1;
}


static void guessContentType(char result[23], const char* fileEnding)
{	/* guesses Content-Type based only on file extension */
	
	if (strlen(fileEnding) > 4);
	else if (!strcmp(fileEnding, "html") ||
			 !strcmp(fileEnding, "css"))
		sprintf(result, "text/%s", fileEnding);
	else if (!strcmp(fileEnding, "json") ||
			 !strcmp(fileEnding, "xml") ||
			 !strcmp(fileEnding, "pdf") ||
			 !strcmp(fileEnding, "zip"))
		sprintf(result, "application/%s", fileEnding);
	else if (!strcmp(fileEnding, "js"))
		sprintf(result, "application/javascript");
	else if (!strcmp(fileEnding, "tar"))
		sprintf(result, "application/x-tar");
	else if ( !strcmp(fileEnding, "woff") ||
			 !strcmp(fileEnding, "woff2") ||
			 !strcmp(fileEnding, "ttf") ||
			 !strcmp(fileEnding, "otf"))
		sprintf(result, "font/%s", fileEnding);
	else if (!strcmp(fileEnding, "png") ||
			 !strcmp(fileEnding, "gif") ||
			 !strcmp(fileEnding, "jpeg") ||
			 !strcmp(fileEnding, "webp"))
		sprintf(result, "image/%s", fileEnding);
	else if (!strcmp(fileEnding, "jpg"))
		sprintf(result, "image/jpeg");
	else if (!strcmp(fileEnding, "svg"))
		sprintf(result, "image/svg+xml");
	else if (!strcmp(fileEnding, "ico"))
		sprintf(result, "image/x-icon");
	else if (!strcmp(fileEnding, "ogg") ||
			 !strcmp(fileEnding, "wav"))
		sprintf(result, "audio/%s", fileEnding);
	else if (!strcmp(fileEnding, "mp3"))
		sprintf(result, "audio/mpeg");
	else if (!strcmp(fileEnding, "m4a"))
		sprintf(result, "audio/mp4");
	else if (!strcmp(fileEnding, "mp4") ||
			 !strcmp(fileEnding, "webm"))
		sprintf(result, "video/%s", fileEnding);
	else if (!strcmp(fileEnding, "mkv"))
		sprintf(result, "video/x-matroska");
}


unsigned httpserver_sendProcessedFile(
	const char* filename, char* contentType, 
	void(*preprocess)(char**, unsigned long*, const char*, const char*))
{
	if (filename[strlen(filename)-1] == '/')
		return 1; /* directories are not valid */

	FILE* file;
	if (!(file = fopen(filename, "rb")))
		return 1;

	fseek(file, 0, SEEK_END);
	unsigned long fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* fileBuffer = malloc(sizeof(char) * fileLen + 1);

	if (!fileBuffer)
	{
		fprintf(stderr, "MEMORY ALLOCATION FAILURE when attempting to read file \"%s\".", filename);
		fclose(file);
		return 2;
	}

	if (!fread(fileBuffer, fileLen, 1, file))
	{
		fprintf(stderr,
			"MEMORY ALLOCATION FAILURE when attempting to read file \"%s\".\n"
			"While allocating file buffer.\n", filename);
		fclose(file);
		return 3;
	}
	
	fclose(file);

	fileBuffer[fileLen] = '\0';


	bool contentTypeMustBeFreed = !contentType;
	if (!contentType)
	{	/* alloc content type and guess it */
		contentType = malloc(sizeof(char) * 23);
		if (!contentType)
		{
			fprintf(stderr,
				"MEMORY ALLOCATION FAILURE when attempting to read file \"%s\".\n"
				"While allocating content type.\n", filename);
			free(fileBuffer);
			return 4;
		}

		int fileEndingStart = 0;

		for (int i = 0; filename[i] && filename[i + 1]; ++i)
			if (filename[i] == '.')
				fileEndingStart = i + 1;

		guessContentType(contentType, filename + fileEndingStart);
	}

	if (preprocess)
		preprocess(&fileBuffer, &fileLen, filename, contentType);

	unsigned fileLenAsStringLen = 2;

	{
	int temp = fileLen;
	while (temp > 10)
	{
		temp /= 10;
		fileLenAsStringLen++;
	}
	}
		
	char* fileLenAsString = malloc(sizeof(char) * fileLenAsStringLen);
	if (!fileLenAsString)
	{
		fprintf(stderr,
			"MEMORY ALLOCATION FAILURE when attempting to read file \"%s\".\n"
			"While allocating file length descriptor.\n", filename);
		free(fileBuffer);
		if (contentTypeMustBeFreed)
			free(contentType);
		return 5;
	}
	sprintf(fileLenAsString, "%ld", fileLen);

	char* header = malloc(sizeof(char) * (fileLen + strlen(HTTP_HEADER_OK)-4 + 
				fileLenAsStringLen + strlen(contentType)));
	if (!header)
	{
		fprintf(stderr,
			"MEMORY ALLOCATION FAILURE when attempting to read file \"%s\".\n"
			"While allocating http header.\n", filename);
		free(fileBuffer);
		if (contentTypeMustBeFreed)
			free(contentType);
		free(fileLenAsString);
		return 6;
	}

	sprintf(header, HTTP_HEADER_OK, contentType, fileLen);

#ifdef _VERBOSE
	printf("Returning:\n\n%s.."
		"and contents of file \"%s\"..\n\n"
		"- - - - - - - -\n\n", header, filename);
#endif
    	send(client_socket, header, strlen(header), 0);
    	send(client_socket, fileBuffer, fileLen, 0);

	free(fileBuffer);
	if (contentTypeMustBeFreed)
		free(contentType);
	free(fileLenAsString);
	free(header);

	return 0;
}

bool httpserver_sendRedirect(const char* redirect)
{
	if (!redirect)
		return 0;

	char* response = malloc(sizeof(char) * strlen(HTTP_HEADER_REDIRECT) - 2 + strlen(redirect));
	if (!response)
		return 0;

	sprintf(response, HTTP_HEADER_REDIRECT, redirect);

	bool result = httpserver_sendString(response);

	free(response);

	return result;
}

bool httpserver_getClientIpv4(unsigned char addr[4])
{
	if (!server_initialized || !client_socket_open || !addr)
		return 0;

	if (client_address.sin_family != AF_INET)
	{
		printf("%d\n", client_address.sin_family);
		return 0;
	}

	addr[0] = (client_address.sin_addr.s_addr      ) & 0xFF;
	addr[1] = (client_address.sin_addr.s_addr >>  8) & 0xFF;
	addr[2] = (client_address.sin_addr.s_addr >> 16) & 0xFF;
	addr[3] = (client_address.sin_addr.s_addr >> 24) & 0xFF;

	return 1;
}

bool httpserver_getClientIpv4String(char buffer[17])
{
	unsigned char addr[4];

	if(!httpserver_getClientIpv4(addr))
		return 0;

	snprintf(buffer, 16, "%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
	buffer[16] = '\0';

	return 1;
}



