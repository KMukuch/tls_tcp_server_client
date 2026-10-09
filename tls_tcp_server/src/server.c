#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include "server.h"

int server_init(struct pollfd* fd_array, int fd_length)
{
	// server file descriptor
	int server_fd;
	// create IPv4, TCP style byte stream, use the default protocol for this socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
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
		
		return -1;
	}
	
	printf("socket created: %d\n", server_fd);
	
	// associate socket with the IP address and port
	if(bind(server_fd, (struct sockaddr *)&server_addr,sizeof server_addr) == -1)
	{
		perror("bind");
		
		return -1;
	}
	
	printf("socket bound to port 8080\n");
	
	if(listen(server_fd, 5) == -1)
	{
		perror("listen");
		
		return -1;
	}
	
	printf("server is listening  on port 8080\n");

	return server_fd;
}

int server_accept_client(int server_fd)
{
	// client file descriptor
	int client_fd;
    
	client_fd = accept(server_fd, NULL, NULL);
	if(client_fd == -1)
	{
		perror("client");
		
		return -1;
	}
	
	printf("client connected\n");
	
	return client_fd;
}

int server_recv(int fd, char* buffer, int len)
{
	int bytes_read = 1;

    // receive bytes from client_fd
    bytes_read = recv(fd, buffer, len, 0);
    if(bytes_read == -1)
    {
        perror("recv");
        
        return 0;
    }
    
	return bytes_read;
}

int server_send(int fd, char* buffer, int len)
{
	if(send(fd, buffer, len, 0) == -1)
	{
		perror("send");

		return 0;
	}

	return 1;
}

int server_close_fds(int* fd_array, int len)
{
    for(int i = 0; i < len; i++)
    {
        close(fd_array[i]);
    }

    return 1;
}