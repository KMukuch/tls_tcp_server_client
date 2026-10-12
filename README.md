# Blocking TLS TCP server and client in C using POSIX Socket API and OpenSSL

This is a collection of simple server and client programs written in C only for educational purposes using OpenSSL for a secure connection.

## TCP send and receive client

A basic TCP client that connects to a server and sends and receivs messages using OpenSSL to create a secure connection.

## Blocking TCP broadcast server using `poll()`

A multi-client TCP server that receives messages from one client and broadcasts them to the other connected clients.

Uses `poll()` to monitor the server socket and multiple client sockets using an array of `struct pollfd`.

## Generating a certificate and keys

To generate a cerrtificate using OpenSSL type:

```
openssl req -x509 -newkey rsa:2048 -nodes -keyout server.key -out server.crt -days 365 -subj "/CN=localhost" -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

This creates:

* `server.key` — private key; do not share it.

* `server.crt` — certificate clients can use to verify the server.

https://docs.openssl.org/master/man7/ossl-guide-tls-introduction/#what-is-tls