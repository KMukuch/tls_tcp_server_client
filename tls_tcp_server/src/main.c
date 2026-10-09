#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include "server.h"
#include "tls.h"

#define MAX_BUFFER 1024
#define MAX_CLIENTS 8

void init_fd_array(struct pollfd* fd_array, int fd_length);

int main(int argc, char *argv[])
{
    SSL_CTX *ssl_ctx = tls_init();
    if(ssl_ctx == NULL)
    {
        return 0;
    }
    
    int fd_counter = 0;
    struct pollfd fd_array[MAX_CLIENTS];

	init_fd_array(&fd_array[0], MAX_CLIENTS);

    int server_fd = server_init(fd_array, MAX_CLIENTS);
    if (server_fd == -1)
    {
        SSL_CTX_free(ssl_ctx);
        
        return 0;
    }

	fd_array[0].fd = server_fd;
	fd_array[0].events = POLLIN;
	fd_counter++;

    SSL *ssl_array[MAX_CLIENTS] = {0};

    while(1)
    {

    }

    return 1;
}

void init_fd_array(struct pollfd* fd_array, int fd_length)
{
	if(fd_length < 0)
	{
		perror("fd_length");
		
		return;
	}
	
	for(int i = 0; i < fd_length; i ++)
	{
		fd_array[i].fd = -1;
		fd_array[i].events = POLLIN;
		fd_array[i].revents = 0;
	}
}