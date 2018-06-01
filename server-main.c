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
	int max_recv_size = 8000, timeout = -1;
	char * server_name = "UDP ECHO Server",
			* server_address = "localhost",
			* server_port =    "12301"
	;

	switch (argc){
		case 5: max_recv_size = (int)strtoul(argv[4],NULL,10);
		case 4: server_port = argv[3];
		case 3: server_address = argv[2];
		case 2: server_name = argv[1];
		case 1: break;

		default:
			fprintf(stderr, "Usage: %s [server name] [hostname | ip address] [port] [max num bytes to receive]\n", argv[0]);
			exit(1);
	}

	struct udphost my_server = createServer(server_name, server_address, server_port);

	socklen_t ip_size = 1024, port_size = 128;
	char  my_ip[ip_size], my_port[port_size];

	getnameinfo(my_server.self->ai_addr, sizeof(struct sockaddr),my_ip, ip_size, my_port, port_size, NI_DGRAM);
	fprintf(stderr, "*** Launching %s on %s:%s ***\n"
			        "[Running...]\n"
			        "[Waiting for incoming connections...]\n\n",
	                my_server.self->ai_canonname, my_ip, my_port)
	;

	struct sockaddr sender;
	char * recv_msg;

	while (1){
		recv_msg = acceptdata(my_server, &sender, timeout, max_recv_size);
		if ( returnto(my_server, recv_msg, sender) < 0 ){
			perror("sendto err: returnto() failed.");
			break;
		}
	}
	shutdown(my_server.socket, 2);
	return errno;
}
