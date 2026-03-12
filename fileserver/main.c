
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../http.h"

#define DEFAULT_PORT 8080
#define DEFAULT_SERVER_CONTENT_PATH "content/"
#define READ_BUFFER_LEN 1024

int main(int argc, char* argv[])
{
	/* 0. parse arguments */
	unsigned portNumber = DEFAULT_PORT;
	char* serverContentPath = DEFAULT_SERVER_CONTENT_PATH;
	for (int i = 1; i < argc; ++i)
	{ 	
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
		{
			printf("This program runs a basic http file server.\n"
			       "Optional parameters:\n"
			       "\t-p / --port <port number> specify the server's port, by default it is %d.\n"
			       "\t-c / --content <path to content folder> specify the path to the server content, by default it is %s.\n",
			       DEFAULT_PORT, DEFAULT_SERVER_CONTENT_PATH);
			return 0;
		}
		else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
		{
			if (i == argc - 1)
				fprintf(stderr, "--port expects a second argument. Ignoring\n");
			if (!(portNumber = (atoi(argv[i + 1]))))
			{
				fprintf(stderr,
				"Provided port number is not an integer; "
				"defaulting to \"%d\".\n", DEFAULT_PORT);
				portNumber = DEFAULT_PORT;
			}
			else
				printf("Setting port number: %d\n", portNumber);
		}
		else if (!strcmp(argv[i], "-c") ||
			 !strcmp(argv[i], "--content"))
		{
			if (i == argc - 1)
				fprintf(stderr, "--content expects a second argument. Ignoring\n");
			else
			{
				printf("Setting content path: %s\n", argv[i+1]);
				serverContentPath = argv[i + 1];
			}
		}
	}

	/* 1. initialize server */
	unsigned fail = httpserver_init(portNumber);
	if (fail)
	{
		fprintf(stderr, "CSERVER INIT FAILED with errno %d\n", fail);
		return EXIT_FAILURE;
	}
	printf("port no.: %d, content path: %s\n", portNumber, serverContentPath);

	/* 2. serve */
	char readBuffer[READ_BUFFER_LEN];
	while(!httpserver_receive(readBuffer, READ_BUFFER_LEN))
	{
		switch(httpserver_getHttpRequestMethod(readBuffer))
		{
		case HTTP_REQUEST_GET:
		{
			char* filepath = readBuffer + 5;

			int i = 0;
			for (; filepath[i]; ++i)
				if (filepath[i] == ' ')
				{
					filepath[i] = '\0';
					/* we only care about the file request for this simple case
					*  cut off everything after the file request */
					break;
				}

			if (!i)
			{
				httpserver_sendRedirect("index.html");
				continue;
			}

			char filename[i + sizeof(serverContentPath)];
			sprintf(filename, "%s%s", serverContentPath, filepath);
			
			if(!httpserver_sendFile(filename, NULL))
				break; /* file successfully sent 
			 * else: file send failure,
			 * switch case continues to the sendError below */
		}
		default:
			httpserver_sendError();
		}
	}

	
	/* 3. terminate server */
	httpserver_kill();
	printf("Server Closed.\n");

	return EXIT_SUCCESS;
}

