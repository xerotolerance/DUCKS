//
// Created by xero on 9/14/17.
//
#include <stdio.h>
#include <netdb.h>
#include "udputils.h"

struct udphost createServer(char * server_name, char * server_address, char * server_port){
	size_t namesize = strlen(server_name);
	struct udphost * self = calloc(sizeof(struct udphost), sizeof(struct udphost));

	struct addrinfo *init_info = calloc(sizeof(struct addrinfo),sizeof(struct addrinfo));

	init_info->ai_family=AF_UNSPEC;
	init_info->ai_socktype=SOCK_DGRAM;
	init_info->ai_flags = AI_PASSIVE;

	self->socket = socket(PF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

	getaddrinfo(server_address, server_port, init_info, &self->self);


	int yes=1;

	
	// lose the pesky "Address already in use" error message
	if (setsockopt(self->socket,SOL_SOCKET,SO_REUSEPORT | SO_REUSEADDR,&yes,sizeof(yes)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	int s;
	s = bind(self->socket, self->self->ai_addr, sizeof(struct sockaddr));
	if (s) {
		//errno = s;
		perror("bind");
		exit(errno);
	}

	self->self->ai_canonname = calloc(namesize,namesize);
	strcpy(self->self->ai_canonname, server_name);

	freeaddrinfo(init_info);
	return *self;
}

struct udphost createClientOnPort(char * client_name, char * client_address, char * client_port){
	struct udphost *self = calloc(sizeof(struct udphost),sizeof(struct udphost));

	struct addrinfo *init_info = calloc(sizeof(struct addrinfo),sizeof(struct addrinfo));


	init_info->ai_family=AF_UNSPEC;
	init_info->ai_socktype=SOCK_DGRAM;
	init_info->ai_flags = 0;

	self->socket = socket(PF_INET, SOCK_DGRAM, 0);

	getaddrinfo(client_address, client_port, init_info, &self->self);

	bind(self->socket, self->self->ai_addr, sizeof(struct sockaddr));

	self->self->ai_canonname = calloc(strlen(client_name),strlen(client_name));
	strcpy(self->self->ai_canonname, client_name);

	freeaddrinfo(init_info);
	return *self;
}

struct udphost createClient(char * client_name, char * client_address){
	size_t namesize = strlen(client_name);
	struct udphost *self = calloc(sizeof(struct udphost),sizeof(struct udphost));

	struct addrinfo *init_info = calloc(sizeof(struct addrinfo),sizeof(struct addrinfo));

	init_info->ai_family=AF_UNSPEC;
	init_info->ai_socktype=SOCK_DGRAM;
	init_info->ai_flags = 0;

	self->socket = socket(PF_INET, SOCK_DGRAM, 0);

	int s =getaddrinfo(client_address, NULL, init_info, &self->self);
	if (s) errno = s, perror(gai_strerror(s));

	self->self->ai_canonname = calloc(namesize, namesize);
	strcpy(self->self->ai_canonname,client_name);

	freeaddrinfo(init_info);
	return *self;
}

int greet(struct udphost my_client, struct sockaddr *srv){
	char * greeting = "Hello from ";
	char msg[strlen(greeting) + strlen(my_client.self->ai_canonname) + 1];

	memcpy(msg, greeting, strlen(greeting));
	strcat(msg, my_client.self->ai_canonname);
	strcat(msg, "!");

	ssize_t bytes_sent = 0;

	for (int i = 0; bytes_sent < strlen(msg) && i < 3; i++) {
		if (bytes_sent < 0)
			perror(strerror(errno));
		else if (bytes_sent == strlen(msg))
			break;
		else
			bytes_sent = sendto(my_client.socket, msg, strlen(msg), 0, srv, sizeof(struct sockaddr));
	}

	return (int)bytes_sent;
}

int returnto(struct udphost my_server, char * recv_msg, struct sockaddr sender){
	ssize_t bytes_sent = 0;
	//char mout[strlen("You said: \"\"")+strlen(recv_msg)];
	if (recv_msg) {
		/*
		strcpy(mout, "You said: \n|\t\"");
		strcat(mout, recv_msg);
		strcat(mout, "\"");
		*/
		for (int i = 0; bytes_sent < strlen(recv_msg) && i < 3; i++)
			if (bytes_sent < 0)
				perror(strerror(errno));
			else if (bytes_sent == strlen(recv_msg))
				break;
			else
				bytes_sent = sendto(my_server.socket, recv_msg, strlen(recv_msg), 0, &sender, sizeof(sender));
	}
	return (int)bytes_sent;
}

char *acceptdata(struct udphost my_server, struct sockaddr *source, int timeout, int datalimit) {
	size_t buf_size = datalimit >= 0? (size_t)datalimit : 1024;
	char recv_data [buf_size];

	socklen_t recv_addr_size = sizeof(source);

	struct pollfd fd;
	fd.fd=my_server.socket;
	fd.events = POLLIN;

	ssize_t eos = -1;
	if (poll(&fd, 1, timeout) > 0)
		eos = recvfrom(my_server.socket, recv_data, buf_size, 0, source, &recv_addr_size);
	else
		return NULL;

	if (eos < 0) {
		fprintf(stderr, "oh fuck.");
		errno = (int)eos, perror(gai_strerror(errno));
		exit(1);
	}
	else {
		recv_data[eos] = '\0';

		socklen_t hostlen = 1024, servicelen = 128;
		char host[hostlen], service[servicelen];
		getnameinfo(source, recv_addr_size, host, hostlen, service, servicelen, NI_DGRAM);

		fprintf(stdout,
		        "[Received inbound message of %d bytes from %s:%s!]\n"
		        "\n+===: System message from %s :====+\n"
				  "|Received %d bytes from %s:%s ...\n"
				  "| |\n"
				  "| +------> %s\n"
				  "+==============================================+\n\n",
		        (int)eos, host, service,
		        my_server.self->ai_canonname, (int)eos, host, service, recv_data
		);
	}

	char * me = calloc((size_t)eos, (size_t)eos);
	strcpy(me, recv_data);

	return me;
}
