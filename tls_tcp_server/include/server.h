#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>

int server_init(struct pollfd* fd_array, int fd_length);

int server_accept_client(int server_fd);

int server_add_client(struct pollfd *fd_array, int fd_counter, int client_fd);

int server_recv(int fd, char* buffer, int len);

int server_send(int fd, char* buffer, int len);

int server_client_shutdown(struct pollfd *fd_array, int client_index, int *fd_counter);

int server_close_fds(int* fd_array, int len);

#endif