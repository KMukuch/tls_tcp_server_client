#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

int server_init(struct pollfd* fd_array, int fd_len);

int server_accept_client(int server_fd);

int server_add_client(struct pollfd *fd_array, int fd_array_size, int client_fd);

int server_recv(int fd, char* buffer, int len);

int server_send(int fd, char* buffer, int len);

void server_client_shutdown(struct pollfd *fd_array, int index);

int server_close_fds(struct pollfd *fd_array, int len);

#endif