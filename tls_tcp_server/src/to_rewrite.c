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

#define MAX_CLIENTS 8
#define MAX_BUFFER 1024

void init_fd_array(struct pollfd* fd_array, int fd_length);

int main(int argc, char **argv)
{
	// OpenSSl section
	// SSL_CTX_new() creates shared server TLS configuration.
	SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	if (ctx == NULL) 
	{
		ERR_print_errors_fp(stderr);

		return 1;
	}
	// SSL_CTX_use_certificate_chain_file() loads the certificate.
	// SSL_CTX_use_PrivateKey_file() loads the private key.
	// SSL_CTX_check_private_key() checks that the key matches the certificate.
	if (SSL_CTX_use_certificate_chain_file(ctx, "server.crt") != 1 || SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) != 1 || SSL_CTX_check_private_key(ctx) != 1)
	{
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);

		return 1;
	}

	// SSL array
	SSL *ssl_array[MAX_CLIENTS];
	
	// server file descriptor
	int server_fd;
	// create IPv4, TCP style byte stream, use the default protocol for this socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	int fd_counter = 0;
	
	struct pollfd fd_array[MAX_CLIENTS];
	
	// create an address structure
	struct sockaddr_in server_addr;
	// IPv4
	server_addr.sin_family = AF_INET;
	// port  8080 (can be any)
	// htons converts 8080 into specific byte order
	server_addr.sin_port  = htons(8080);
	// Use any IPv4 network interface (wire,WiFi ..)
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		
	if(server_fd == -1)
	{
		perror("socket");
		
		return 1;
	}
	
	printf("socket created: %d\n", server_fd);
	
	// associate socket with the IP address and port
	if(bind(server_fd, (struct sockaddr *)&server_addr,sizeof server_addr) == -1)
	{
		perror("bind");
		
		return 1;
	}
	
	printf("socket bound to port 8080\n");
	
	if(listen(server_fd, 5) == -1)
	{
		perror("listen");
		
		return 1;
	}
	
	printf("server is listening  on port 8080\n");
	
	// using an array of pollfd
	init_fd_array(&fd_array[0], MAX_CLIENTS);
	fd_array[0].fd = server_fd;
	fd_array[0].events = POLLIN;
	fd_counter++;
	
	while(1)
	{
		// detect an activity
		int ready = poll(fd_array, fd_counter, -1);
		if(ready == -1)
		{
			perror("poll");
			
			return 1;
		}
		
		for(int i = 0; i < fd_counter; i++)
		{
			if(fd_array[i].revents & POLLIN)
			{
				if(fd_array[i].fd == server_fd)
				{
					int client_fd = accept(server_fd, NULL, NULL);
					if(client_fd == -1)
					{
						perror("accept");
					}
					else if (fd_counter < MAX_CLIENTS)
					{
						SSL *ssl = SSL_new(ctx);
						if(ssl == NULL || SSL_set_fd(ssl, client_fd) != 1) 
						{
							ERR_print_errors_fp(stderr);
							SSL_free(ssl);
							close(client_fd);
						}
						else if(SSL_accept(ssl) != 1)
						{
							ERR_print_errors_fp(stderr);

							SSL_free(ssl);
							close(client_fd);
						}
						else
						{
							fd_array[fd_counter].fd = client_fd;
							fd_array[fd_counter].events = POLLIN;
							fd_array[fd_counter].revents = 0;

							ssl_array[fd_counter] = ssl;
							fd_counter++;

							printf("TLS client connected\n");
						}
					}
					else
					{
						close(client_fd);
					}
					
					printf("new client: %d\n", client_fd);
				}
				else
				{
					char buffer[MAX_BUFFER];
					
					int bytes_read = SSL_read(ssl_array[i], buffer, sizeof(buffer) - 1);
					if(bytes_read > 0)
					{
						buffer[bytes_read] = '\0';
						printf("client %d: %s", fd_array[i].fd, buffer);
						
						for(int k = 0; k < fd_counter; k++)
						{
							if(k != i && fd_array[k].fd != server_fd)
							{
								int n = SSL_write(ssl_array[k], buffer, bytes_read);
								if (n <= 0)
								{
									int err = SSL_get_error(ssl_array[k], n);
									if(err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
									{
										ERR_print_errors_fp(stderr);
									}
								}
							}
						}
					}
					else if(bytes_read == 0)		
					{
						printf("client disconnected: %d\n", fd_array[i].fd);
						
						close(fd_array[i].fd);

						SSL_free(ssl_array[i]);
						
						for(int j = i; j < fd_counter - 1; j++)
						{
							fd_array[j] = fd_array[j+1];
							ssl_array[j] = ssl_array[j + 1];
						}
						fd_counter--;
						i--;
					}
					else if(bytes_read == -1)		
					{
						int err = SSL_get_error(ssl_array[i], bytes_read);
						if(err == SSL_ERROR_ZERO_RETURN)
						{
							// The TLS peer closed the connection cleanly.
						}
						else if(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
						{
							// Retry when poll indicates the appropriate readiness.
						}
						else
						{
							// Fatal TLS or underlying I/O error.
							ERR_print_errors_fp(stderr);
						}
					}
				}
			}
			else if(fd_array[i].revents & POLLHUP)		
			{
				printf("client disconnected: %d\n", fd_array[i].fd);
				
				close(fd_array[i].fd);

				SSL_free(ssl_array[i]);
				
				for(int j = i; j < fd_counter - 1; j++)
				{
					fd_array[j] = fd_array[j+1];
					ssl_array[j] = ssl_array[j + 1];
				}
				fd_counter--;
				i--;
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

	SSL_CTX_free(ctx);
	close(server_fd);
	
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