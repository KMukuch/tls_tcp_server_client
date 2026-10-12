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
        return 1;
    }

    SSL *ssl_array[MAX_CLIENTS] = {0};


    struct pollfd fd_array[MAX_CLIENTS];
    init_fd_array(fd_array, MAX_CLIENTS);

    int server_fd = server_init(fd_array, MAX_CLIENTS);
    if (server_fd == -1)
    {
        tls_close(ssl_ctx);

        return 1;
    }

	while(1)
	{
		int ready = poll(fd_array, MAX_CLIENTS, -1);
		if(ready == -1)
		{
			perror("poll");
			
            server_close_fds(fd_array, MAX_CLIENTS);
            tls_close(ssl_ctx);

			return 1;
		}
		
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(fd_array[i].revents & POLLIN)
			{
				if(fd_array[i].fd == server_fd)
				{
					int client_fd = server_accept_client(server_fd);
                    if(client_fd != -1)
                    {
                        SSL *ssl_client = tls_accept_client(ssl_ctx, client_fd);
                        if(ssl_client != NULL)
                        {
                            int index = server_add_client(fd_array, MAX_CLIENTS, client_fd);
                            if(index != -1)
                            {
                                tls_add_client(ssl_array, index, ssl_client);
                            }
                            else
                            {
                                SSL_free(ssl_client);
                            }
                        }
                    }
				}
				else
				{
					char buffer[MAX_BUFFER];
					
					int bytes_read = tls_read(ssl_array[i], buffer, MAX_BUFFER);
					if(bytes_read > 0)
					{
						printf("client %d: %s\n", fd_array[i].fd, buffer);
						
						for(int k = 0; k < MAX_CLIENTS; k++)
						{
							if(k != i && fd_array[k].fd != -1 && ssl_array[k] != NULL)
							{
								tls_write(ssl_array[k], buffer, bytes_read);
							}
						}
					}
					else if(bytes_read == 0)		
					{
                        tls_client_shutdown(ssl_array, i);
                        server_client_shutdown(fd_array, i);
					}
				}
			}
			else if(fd_array[i].revents & POLLHUP)		
			{
                tls_client_shutdown(ssl_array, i);
                server_client_shutdown(fd_array, i);
			}
			else if(fd_array[i].revents & POLLERR)		
			{
				perror("poll");
			}
			else if(fd_array[i].revents & POLLNVAL)		
			{
				perror("poll");
			}
		}
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