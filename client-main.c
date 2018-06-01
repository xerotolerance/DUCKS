//
// Created by xero on 9/14/17.
//

#include <stdio.h>
#include "udputils.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>


int main(int argc, char ** argv){
	if (argc != 4) {
		fprintf(stderr, "Usage: %s [host | IP Address] [port] [length of message]\n\n"
				        "\t\tExamples: %s google.com 80 7\n"
				        "\t\t          %s 127.0.0.1 12301 27\n"
				        "\t\t          %s github.com/xerotolerance 443 420\n\n",
		        argv[0], argv[0], argv[0], argv[0]
		);
		exit(1);
	}

	char * client_name = "UDP ECHO Client",
			* client_address = "127.0.0.1"
	;

	struct udphost my_client = createClient(client_name, client_address);

	socklen_t s_ip_size = 1024, c_ip_size = 1024,
			  s_port_size = 128, c_port_size = 128;

	char  my_ip[s_ip_size], my_port[s_port_size],
		  dest_ip[s_ip_size], dest_port[s_port_size];

	struct addrinfo *dest;
	getaddrinfo(argv[1], argv[2], NULL, &dest);
	dest->ai_socktype=SOCK_DGRAM;

	unsigned long bytes_to_send = strtoul(argv[3],NULL,10);

	char data[bytes_to_send];
	memset(data, '>', bytes_to_send);

	getnameinfo(my_client.self->ai_addr, sizeof(struct sockaddr), my_ip, c_ip_size, my_port, c_port_size, NI_DGRAM);
	getnameinfo(dest->ai_addr, sizeof(struct sockaddr),dest_ip, s_ip_size, dest_port, s_port_size, NI_DGRAM);

	fprintf(stderr, "*** Launching %s on %s:%s ***\n"
			        "[Running...]\n"
			        "[Sending outbound message of %lu bytes to %s:%s...]\n",
	         my_client.self->ai_canonname, my_ip, my_port, bytes_to_send, dest_ip, dest_port);
	;

	/*
	if ( greet(my_client, dest->ai_addr) < 0 )
		perror("sendto err: greet() failed."), exit(errno);
	*/

	ssize_t bytes_sent = sendto(my_client.socket, data, bytes_to_send, 0, dest->ai_addr, sizeof(struct sockaddr));


	if (bytes_sent < 0)
		perror("\nsendto err: request send failed."), exit(errno);
	else
		fprintf(stderr, "\n[Message sent successfully.]\n"
				        "\n[Awaiting ECHO response...]\n");

	struct sockaddr server;
	char * rts_msg;
	int timeout = 1000;
	for (int tries = 0, max_num_attempts = 3; tries < max_num_attempts && !(rts_msg = acceptdata(my_client, &server, timeout, -1)); tries++) {
		 fprintf(stderr, "(%d) Return message not received within %.3f seconds, retrying... \n", tries+1, (double)(timeout/1000));
	}

	if (!rts_msg)
		fprintf(stderr, "\nDid not receive response from %s:%s in time."
				        "\n\tERR: E_TIMEOUT"
				        "\n Is the server running on port %s?\n", argv[1], argv[2], argv[2]);
	else
		fprintf(stderr, "[Successfully received ECHO response from %s:%s!]\n"
				        "[Closing connection to server.]\n"
				        "[Shutting down client..]\n"
		                "[GoodBye.]\n", dest_ip, dest_port
		);
	shutdown(my_client.socket, 2);
	freeaddrinfo(dest);

	return rts_msg ? 0 : 1;
}
