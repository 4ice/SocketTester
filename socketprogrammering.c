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
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <linux/if_link.h>

void server();
void client();
void readInput();
char **getCommand();
void cmdClientHelp();
void cmdServerHelp();
char *getIp();
void sendPacket(int protocol, char *host, int portno, int packetSize, long fileSize);
void recieveMeessage(int protocol, int portno, int size);


//for the use of bool
typedef int bool;
#define true 1;
#define false 0;



void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if(argc < 2) {
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
	printf("this is a server!!\nNow tell me what you want to do, if you need help, write \"--help\"\n");
	char **commands;
	commands = getCommand();


	int i = 0;
	int nrOfCommands = 0;
	bool quit = false;

	while(!quit && commands[i] != NULL)
	{
		nrOfCommands++;
		//help
		if(strcmp(commands[i], "--help\n") == 0)
		{
			cmdServerHelp();		
			i++;
		}
		//Listen
		else if(strcmp(commands[i], "--listen") == 0)
		{
			i++;
			if(commands[i+2] == NULL)
			{
				error("ERROR: to few parameters");
				commands[i] = NULL;
			}
			else
			{
				//TCP
				if(strcmp(commands[i], "t") == 0)
				{
					recieveMeessage(1, strtol(commands[i+1], NULL, 10), strtol(commands[i+2], NULL, 10));
					i = i+2;
				}
				else if(strcmp(commands[i], "u") == 0)
				{
					recieveMeessage(0, strtol(commands[i+1], NULL, 10), strtol(commands[i+2], NULL, 10));
					i = i+2;
				}
				else
					error("ERROR: not a valid protocol");
			}

		}
	 	else if(strcmp(commands[i], "--quit\n") == 0)
	    	{
		    	quit = true;
		    	exit(0);
	    	}
	    	//unknown command
	    	else
	    	{
		    	printf("Unknown command, please use '--help' for help\n");
		    	commands[i] = NULL;
	    	}


	    
	    	if(commands[i] == NULL && !quit)
	    	{
		    	//Free memory
		    	/*int k = 0;
		    	for(k; k < nrOfCommands; k++)
		    	{
			    	free(commands[k]);
		    	}
		    	free(commands);
		    	*/
		    	commands = getCommand();
		    	i = 0;
		    	nrOfCommands = 0;
	    	}
	}

}

void client(int argc, char *argv[])
{
    	char input[64];
    	printf("this is a client!!\nNow tell me what you want to do, if you need help, write \"--help\"\n");
    	char **commands;
    	commands = getCommand();


    

    	int i = 0;
    	int nrOfCommands = 0;
    	bool quit = false;
    	while(!quit && commands[i] != NULL)
    	{
	    	nrOfCommands++;
	    	//help
	    	if(strcmp(commands[i], "--help\n") == 0)
	    	{
		    	cmdClientHelp();
		    	i++;
	    	}
	    	//send
	    	else if(strcmp(commands[i], "--send") == 0)
	    	{
		    	i++;		    
		    	if(commands[i+4] == NULL)
		    	{
			    	error("ERROR: to few parameters");
			    	commands[i] = NULL;
		    	}
		    	else
		    	{
			    	//TCP
			    	if(strcmp(commands[i], "t") == 0)
			    	{
				    	sendPacket(1, commands[i+1], strtol(commands[i+2], NULL, 10), strtol(commands[i+3], NULL, 10), strtol(commands[i+4], NULL, 10));
				    	i = i+5;
			    	}
			    	//UDP
			    	else if(strcmp(commands[i], "u") == 0)
			    	{
				    	sendPacket(0, commands[i+1], strtol(commands[i+2], NULL, 10), strtol(commands[i+3], NULL, 10), strtol(commands[i+4], NULL, 10));
				    	i = i+5;
			    	}
			    	else
				    	error("ERROR: not a valid protocol");
					    
		    	}
	    	}
	    	//quit
	    	else if(strcmp(commands[i], "--quit\n") == 0)
	    	{
		   	quit = true;
		    	exit(0);
	    	}
	    	//unknown command
	    	else
	    	{
		    	printf("Unknown command, please use '--help' for help\n");
		    	commands[i] = NULL;
	    	}


	    
	    	if(commands[i] == NULL && !quit)
	    	{
		    	//Free memory
		    	/*int k = 0;
		    	for(k; k < nrOfCommands; k++)
		    	{
			    	free(commands[k]);
		    	}
		    	free(commands);
		    	*/
		    	commands = getCommand();
		    	i = 0;
		    	nrOfCommands = 0;
	    	}
    	}   
}

char **getCommand()
{
    int command_num;
    char input[BUFSIZ];
    char **commandArray;

    command_num = 0;
    commandArray = (char**) malloc (BUFSIZ * sizeof(char));

    fgets(input,sizeof(input),stdin);
    commandArray[command_num] = strtok(input, " "); /*breaks a string of commands into
                                                      tokens*/
    while (commandArray[command_num]!= NULL)
    {
        command_num++;
        commandArray[command_num] = strtok (NULL, " ");
    }

    commandArray[command_num] = NULL;  /*very imp - adds null in the last position*/
    return commandArray;
}



void cmdClientHelp()
{
	printf("****\nHELP\n****\n");
	printf("--help \t\t\t\t\tShows this help text.\n");

	printf("--send [t/u} [hostIP] [port] [size] \tSends a packet to given host, port and packetsize\n");
	printf("\tt \t\t\t\tTCP protocol\n");
	printf("\tu \t\t\t\tUDP protocol\n\n");
	printf("--quit \t\t\t\t\tclose the program\n");
}

void cmdServerHelp()
{
	printf("****\nHELP\n****\n");
	printf("--help \t\t\t\t\tShows this help text.\n");

	printf("--listen [t/u} [port] [size]\t\tListen for packet, given protocol and port\n");
	printf("\tt \t\t\t\tTCP protocol\n");
	printf("\tu \t\t\t\tUDP protocol\n\n");
	printf("--quit \t\t\t\t\tclose the program\n");
}

char *getIp()
{
	char *ipAddress;

	struct ifaddrs *addrs, *tmp;
	getifaddrs(&addrs);
	tmp = addrs;
	while(tmp)
	{
		if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET && strcmp(tmp->ifa_name, "eno1") == 0)
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *) tmp->ifa_addr;
			ipAddress = inet_ntoa(pAddr->sin_addr);
		}
		tmp = tmp->ifa_next;
	}

	freeifaddrs(addrs);

	return ipAddress;
}
//NOTE: TCP == 1, UDP == 0
void sendPacket(int protocol, char *host, int portno, int packetSize, long fileSize)
{
    int sockfd, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;




    char buffer[packetSize];
    memset(buffer, 'a', sizeof(buffer));
    
    //Open a socket, either UDP or TCP
    if(protocol == 0)
    {
	    //UDP
	    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else
    {
	    //TCP
	    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    //Check if the socket could be opened
    if (sockfd < 0)
    {
	    error("ERROR opening socket");
    }

    //Get the server
    server = gethostbyname(host);
    if (server == NULL) {
	    fprintf(stderr, "ERROR, no such host\n");
	    exit(0);
    }
    //Set all values in serv_addr to zero
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //Use IPv4 Internet protocols
    serv_addr.sin_family = AF_INET;

    //copy server->h_addr to serv_addr.sin_addr.s_addr
    bcopy((char *)server->h_addr,
		    (char *)&serv_addr.sin_addr.s_addr,
       		    server->h_length);
    //Add the port number
    serv_addr.sin_port = htons(portno);
    //Try to connect to the host
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	    error("ERROR connecting");
    while(fileSize > 0)
    {
	    n = write(sockfd, buffer, strlen(buffer));
	    if(n < 0)
		    error("ERROR writing to socket");
	    fileSize = fileSize-packetSize;
    }
    n = write(sockfd, "b", 1);
    if(n < 0)
	    error("ERROR writing to socket");

    //Close the connection
    close(sockfd);
}

void recieveMeessage(int protocol, int portno, int size)
{
	time_t ticks;

    	printf("-----Server information-----\n");

	struct ifaddrs *addrs, *tmp;

	getifaddrs(&addrs);
	tmp = addrs;
	while(tmp)
	{
		if(tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
		{
			struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
		        printf("%s:\t\t%s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
		}
		tmp = tmp->ifa_next;
	}
	freeifaddrs(addrs);
    	char *protocolName;
    	if(protocol == 0)
	    	protocolName = "UDP";
    	else
	    	protocolName = "TCP";


    	printf("Protocol:\t%s\n", protocolName);
	printf("Port:\t\t%i\n", portno);
	printf("PacketLength\t%i\n", size);
	printf("----------------------------\n");


	int sockfd, newsockfd, clilen;
        char buffer[size];
        struct sockaddr_in serv_addr, cli_addr;
        int n;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
           error("ERROR opening socket");
	
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
                 sizeof(serv_addr)) < 0) 
                 error("ERROR on binding");
        
	listen(sockfd,5);
        
	clilen = sizeof(cli_addr);
	ticks = time(NULL);
	bool keepListen = true;
	while(keepListen)
	{
        	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
		memset(buffer, 0, size);
        	n = read(newsockfd,buffer,size-1);
		if(strcmp(buffer, "b") == 0)
		{
			keepListen = false;
		}
		printf("%s\n\n", buffer);
	}

        printf("Here is the message: %s\n",buffer);

	close(newsockfd);
	close(sockfd);
}
