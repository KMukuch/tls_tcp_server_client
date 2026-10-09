#ifndef TLS_H
#define TLS_H

#include <stdio.h>

SSL_CTX* tls_init(void);

SSL* tls_accept_client(SSL_CTX *ctx, int client_fd);

int tls_recv(SSL *ssl, char *buffer, int len);

int tls_send(SSL *ssl, const char *buffer, int len);

void tls_client_shutdown(SSL *ssl);

#endif