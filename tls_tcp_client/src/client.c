#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define CERT_PATH "../data/server.crt"

int main(int argc, char **argv)
{
    char buffer[1024];

    // OpenSSL section
    // TLS_client_method() selects client mode and negotiates a supported TLS version. SSL_CTX_new() creates the shared TLS configuration object.
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if(ctx == NULL)
    {
        ERR_print_errors_fp(stderr);

        return 1;
    }

    //This enables peer certificate verification and loads OpenSSL's configured default trust locations.
    // SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    // if(SSL_CTX_set_default_verify_paths(ctx) != 1)
    // {
    //     ERR_print_errors_fp(stderr);
    //     SSL_CTX_free(ctx);

    //     return 1;
    // }

    // Require verification of the server's certificate.
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    // Trust your specific server certificate.
    if(SSL_CTX_load_verify_locations(ctx, CERT_PATH, NULL) != 1)
    {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }

	int socket_fd;
	// create IPv4, TCP style byte stream, use the default protocol for this socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);		
	if(socket_fd == -1)
	{
		perror("socket");
		
		return 1;
	}
	
    char *ip_addr = argv[1];
    int port = atoi(argv[2]);
	
    // create an address structure
	struct sockaddr_in server_addr;
	// IPv4
	server_addr.sin_family = AF_INET;
	// port  8080 (can be any)
	// htons converts 8080 into specific byte order
	server_addr.sin_port  = htons(port);
	inet_pton(AF_INET, ip_addr, &server_addr.sin_addr);
	
	printf("socket created: %d\n", socket_fd);
	
	// associate socket with the IP address and port
	if(connect(socket_fd, (struct sockaddr *)&server_addr,sizeof server_addr) == -1)
	{
		perror("connect");
		
		return 1;
	}
	
	printf("connected\n");

    // SSL_new() creates the connection object
    SSL *ssl = SSL_new(ctx);
    if(ssl == NULL)
    {
        ERR_print_errors_fp(stderr);
        close(socket_fd);
        SSL_CTX_free(ctx);

        return 1;
    }
    // SSL_set_fd() attaches the existing TCP socket to ssl
    if(SSL_set_fd(ssl, socket_fd) != 1)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(socket_fd);
        SSL_CTX_free(ctx);

        return 1;
    }

    // Verify identity
    // if (X509_VERIFY_PARAM_set1_ip_asc(SSL_get0_param(ssl), ip_addr) != 1)
    // {
    //     ERR_print_errors_fp(stderr);
    //     SSL_free(ssl);
    //     close(socket_fd);
    //     SSL_CTX_free(ctx);
    //     return 1;
    // }
    // Handshake
    if (SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(socket_fd);
        SSL_CTX_free(ctx);

        return 1;
    }
	
	while(1)
	{
		fgets(buffer, sizeof(buffer), stdin);
		printf("sending: %s", buffer);
        
        int n = SSL_write(ssl, buffer, (int)strlen(buffer));

        if (n <= 0)
        {
            int err = SSL_get_error(ssl, n);

            fprintf(stderr, "SSL_write failed, error: %d\n", err);
            ERR_print_errors_fp(stderr);

            return 1;
        }

		// int n = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
		// if (n == -1)
		// {
		// 	perror("recv");

		// 	return 1;
		// }

		// buffer[n] = '\0';

		// printf("received: %s", buffer);
	}
	
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    
	close(socket_fd);

    return 0;
}