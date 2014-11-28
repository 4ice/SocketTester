/*
 * =====================================================================================
 *
 *       Filename:  socketprogrammering.c
 *
 *    Description:  A program for testing TCP/UDP socket-traffic, 
 *    		    this can then be logged with tcpdump or such
 *
 *        Version:  1.0
 *        Created:  2014-11-25 20:46:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  David Andersson 
 *   Organization:  BTH - IT-säk 13
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void server();
void client();
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if(argc < 3) {
		fprintf(stderr, "ERROR: the amount of parameters must be atleast 2");
		exit(1);
	}
	if(strcmp(argv[1], "-s") == 0) {
		server(argc, argv);
	}
	else if(strcmp(argv[1], "-c") == 0) {
		client(argc, argv);
	}
	return EXIT_SUCCESS;
}


void server(int argc, char *argv[])
{
	printf("this is a server!!\n");

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 3) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[2]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,
			(struct sockaddr *) &cli_addr,
			&clilen);
	if (newsockfd < 0)
		error("ERROR on accept");
	bzero(buffer, 256);
	n = read(newsockfd, buffer, 255);
	if (n < 0)
		error("ERROR reading from socket");
	printf("Here is the message: %s\n", buffer);
	n = write(newsockfd, "I got your message", 18);
	if ( n < 0)
		error("ERROR writing to socket");
	close(newsockfd);
	close(sockfd);
}

void client(int argc, char *argv[])
{
	printf("this is a client!!\n");
	
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	if (argc < 4) {
		fprintf(stderr, "some parameter is missing\n");
		exit(0);
	}
	portno = atoi(argv[3]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[2]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
	     (char *)&serv_addr.sin_addr.s_addr,
	     server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
	printf("Please enter the message: ");
	bzero(buffer, 256);
	fgets(buffer, 255, stdin);
	n = write(sockfd, buffer, strlen(buffer));
	if (n<0)
		error("ERROR writing to socket");
	bzero(buffer, 256);
	if (n<0)
		error("ERROR reading from socket");
	printf("%s\n", buffer);
	close(sockfd);
}
