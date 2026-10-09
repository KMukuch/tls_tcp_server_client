#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "tls.h"

SSL_CTX *tls_init(void)
{
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	if (ctx == NULL) 
	{
		ERR_print_errors_fp(stderr);

		return NULL;
	}
	// SSL_CTX_use_certificate_chain_file() loads the certificate.
	// SSL_CTX_use_PrivateKey_file() loads the private key.
	// SSL_CTX_check_private_key() checks that the key matches the certificate.
	if (SSL_CTX_use_certificate_chain_file(ctx, "server.crt") != 1 || SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) != 1 || SSL_CTX_check_private_key(ctx) != 1)
	{
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);

		return NULL;
	}

    return ctx;
}

SSL* tls_accept_client(SSL_CTX *ctx, int client_fd)
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
    
    return ssl;
}

void tls_client_shutdown(SSL *ssl)
{
    
}