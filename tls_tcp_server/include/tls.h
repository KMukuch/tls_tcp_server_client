#ifndef TLS_H
#define TLS_H

#include <stdio.h>

SSL_CTX* tls_init(void);

SSL* tls_accept_client(SSL_CTX *ctx, int client_fd);

void tls_add_client(SSL **ssl_array, int index, SSL *ssl);

int tls_read(SSL *ssl, char *buffer, int buffer_size);

int tls_write(SSL *ssl, const char *buffer, int bytes_read);

void tls_client_shutdown(SSL **ssl_array, int index);

void tls_close(SSL_CTX *ctx);

#endif