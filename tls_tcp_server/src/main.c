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

struct Client
{
    int fd;
    SSL *ssl;
};

void init_fd_array(struct pollfd* fd_array, int fd_length);
void init_client_array(struct Client *clients, int length);

int main(int argc, char *argv[])
{
    SSL_CTX *ssl_ctx = tls_init();
    if(ssl_ctx == NULL)
    {
        return 1;
    }

    struct pollfd fd_array[MAX_CLIENTS];
    init_fd_array(fd_array, MAX_CLIENTS);

    struct Client client_array[MAX_CLIENTS] = {0};
    init_client_array(client_array, MAX_CLIENTS);

    int server_fd = server_init(fd_array, MAX_CLIENTS);
    if (server_fd == -1)
    {
        tls_close(ssl_ctx);

        return 1;
    }
    
    server_close_fds(fd_array, MAX_CLIENTS);
    tls_close(ssl_ctx);

    return 0;
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

void init_client_array(struct Client *client_array, int length)
{
    for (int i = 0; i < length; i++)
    {
        client_array[i].fd = -1;
        client_array[i].ssl = NULL;
    }
}