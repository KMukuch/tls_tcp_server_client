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
    if (ssl == NULL)
    {
        ERR_print_errors_fp(stderr);
        close(client_fd);
        return NULL;
    }

    if (SSL_set_fd(ssl, client_fd) != 1)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        return NULL;
    }

    if (SSL_accept(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        return NULL;
    }

    return ssl;
}

void tls_add_client(SSL **ssl_array, int index, SSL *ssl)
{
    ssl_array[index] = ssl;
}

int tls_read(SSL *ssl, char *buffer, int buffer_size)
{
    int bytes_read = SSL_read(ssl, buffer, buffer_size - 1);

    if(bytes_read > 0)
    {
        buffer[bytes_read] = '\0';

        return bytes_read;
    }

    int err = SSL_get_error(ssl, bytes_read);

    if(err == SSL_ERROR_ZERO_RETURN)
    {
        // Client closed the TLS connection cleanly.
        return 0;
    }

    if(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
    {
        // Retry when the socket is ready.
        return -2;
    }

    ERR_print_errors_fp(stderr);

    return -1;
}

int tls_write(SSL *ssl, const char *buffer, int bytes_read)
{
    int n = SSL_write(ssl, buffer, bytes_read);
    if(n <= 0)
    {
        int err = SSL_get_error(ssl, n);

        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        {
            return -2; // Retry later
        }

        ERR_print_errors_fp(stderr);

        return -1; // Error
    }

    return n;
}

void tls_client_shutdown(SSL **ssl_array, int index)
{
    if (ssl_array[index] != NULL)
    {
        SSL_shutdown(ssl_array[index]);
        SSL_free(ssl_array[index]);
        ssl_array[index] = NULL;
    }
}

void tls_close(SSL_CTX *ctx)
{
    SSL_CTX_free(ctx);
}