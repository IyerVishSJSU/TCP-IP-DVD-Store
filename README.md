# TCP-IP-DVD-Store Readme
Aims at implementing a DVD Store that can concurrently accept clients utilizing either TCP or UDP Protocol
Author : Vishwanath Iyer

Program: Server_Q2.c

 TCP and UDP sockets are created and bind initially. Additional listen call is called for TCP connection.
 FD_SET is used to create bit vector for the file descriptors of TCP and UDP.
 The server then enters an infinite loop in which it calls select() to wait for one or more of the descriptors to become ready. select() is used in the server to support both multiprotocol clients
 If the tcp descriptor tsock becomes ready, then the server calls accept to obtain a new connection. A new TCP slave socket ssock is created to handle the response. A new thread is created to service each new client connection by pthread_create() which calls routine tcp_dvd for handling list and order actions and write call is used to send response back to the client.
 If the udp descriptor usock becomes ready, the server uses recvfrom() call once and then creates a new thread to handle the response request using the pthread_create() and udp_dvd routine is called which works same as tcp_dvd for UDP connections.

Program: Client_Q2_TCP.c

 This program creates a TCP client to implement a virtual DVD Store and send request to the server in an Iterative fashion.
 The client socket file descriptor is created using socket () system call.
 gethostbyname() function returns a pointer to an object of the type hostent structure.
 The strcmp () function compares the argument given by the user in command line as list or order and writes the request using write () call through buffer variable.
 The response from server is read back from buffer using read () call and displayed on the terminal accordingly.

Program: Client_Q2_UDP.c

 This program creates a UDP client to implement a virtual DVD Store and send request to the server in an Iterative fashion.
 The client socket file descriptor is created using socket () system call.
 Structure from is used to receive the response from the UDP Server and display the results.
 gethostbyname() function returns a pointer to an object of the type hostent structure.
 The strcmp () function compares the argument given by the user in command line as list or order and writes the request using sendto () call through buffer variable.
 The response from server is read back from buffer using recvfrom () call and displayed on the terminal accordingly.
