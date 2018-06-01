//
// Created by xero on 9/13/17.
//

#ifndef CS356_PROJECT1_UDPUTILS_H
#define CS356_PROJECT1_UDPUTILS_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>

struct udphost{
	struct addrinfo *self;
	int socket;
};

struct udphost createServer(char *, char *, char *);
struct udphost createClientOnPort(char *, char *, char *);
struct udphost createClient(char *, char *);

int greet(struct udphost, struct sockaddr*);
int returnto(struct udphost, char *, struct sockaddr);

char *acceptdata(struct udphost, struct sockaddr *, int, int datalimit);


#endif //CS356_PROJECT1_UDPUTILS_H
