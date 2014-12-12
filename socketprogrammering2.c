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
#include <time.h>
#include <sys/time.h>
#include<errno.h> //For errno - the error number
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header

void server();
void client();
void readInput();
char **getCommand();
void cmdClientHelp();
void cmdServerHelp();
char *getIp();
void sendPacket(int protocol, char *host, int portno, int packetSize, long fileSize);
void recieveMeessage(int protocol, int portno, int packetSize, long fileSize);



/* 
    96 bit (12 bytes) pseudo header needed for tcp header checksum calculation 
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_length;
};
 
/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    register short answer;
 
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
 
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
     
    return(answer);
}



//for the use of bool
typedef int bool;
#define true 1;
#define false 0;

long int sizeToRecieve;

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
			if(commands[i+3] == NULL)
			{
				error("ERROR: to few parameters");
				commands[i] = NULL;
			}
			else
			{
				//TCP
				if(strcmp(commands[i], "t") == 0)
				{
					recieveMeessage(1, strtol(commands[i+1], NULL, 10), strtol(commands[i+2], NULL, 10), strtol(commands[i+3], NULL, 10));
					i = i+4;
				}
				else if(strcmp(commands[i], "u") == 0)
				{
					recieveMeessage(0, strtol(commands[i+1], NULL, 10), strtol(commands[i+2], NULL, 10), strtol(commands[i+3], NULL, 10));
					i = i+4;
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
	printf("--help \t\t\t\t\t\t\tShows this help text.\n");

	printf("--send [t/u} [hostIP] [port] [packetSize] [fileSize] \tSends a packet to given host, port and packetsize\n");
	printf("\tt \t\t\t\t\t\tTCP protocol\n");
	printf("\tu \t\t\t\t\t\tUDP protocol\n\n");
	printf("--quit \t\t\t\t\t\t\tclose the program\n");
}

void cmdServerHelp()
{
	printf("****\nHELP\n****\n");
	printf("--help \t\t\t\t\tShows this help text.\n");

	printf("--listen [t/u} [port] [packetSize]\tListen for packet, given protocol and port\n");
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
    //Create a raw socket
    int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
     
    if(s == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket");
        exit(1);
    }
     
    //Datagram to represent the packet
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
     
    //zero out the packet buffer
    memset (datagram, 0, 4096);
     
    //IP header
    struct iphdr *iph = (struct iphdr *) datagram;
     
    //TCP header
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
    struct sockaddr_in sin;
    struct pseudo_header psh;
     
    //Data part
    data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);
    memset(data, 'a', 64);
     
    //some address resolution
    strcpy(source_ip , "192.168.0.3");
    sin.sin_family = AF_INET;
    sin.sin_port = htons(6666);
    sin.sin_addr.s_addr = inet_addr ("192.168.0.2"); //Destination?
     
    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct tcphdr) + strlen(data);
    iph->id = htonl (54321); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr ( source_ip );    //Spoof the source ip address
    iph->daddr = sin.sin_addr.s_addr;
     
    //Ip checksum
    iph->check = csum ((unsigned short *) datagram, iph->tot_len);
     
    //TCP Header
    tcph->source = htons (1234);
    tcph->dest = htons (6666);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;  //tcp header size
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (5840); /* maximum allowed window size */
    tcph->check = 0; //leave checksum 0 now, filled later by pseudo header
    tcph->urg_ptr = 0;
     
    //Now the TCP checksum
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data) );
     
    int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data);
    pseudogram = malloc(psize);
     
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr) + strlen(data));
     
    tcph->check = csum( (unsigned short*) pseudogram , psize);
     
    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;
     
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }
     
    //loop if you want to flood :)
    while (1)
    {
        //Send the packet
        if (sendto (s, datagram, iph->tot_len ,  0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
        {
            perror("sendto failed");
        }
        //Data send successfully
        else
        {
            printf ("Packet Send. Length : %d \n" , iph->tot_len);
        }
    }
}

void recieveMeessage(int protocol, int portno, int packetSize, long fileSize)
{

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
	    printf("PacketLength\t%i\n", packetSize);
	    printf("----------------------------\n");


	    int sockfd, newsockfd, clilen;
        char buffer[packetSize];
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
	    bool keepListen = true;
        long int currentlyRecieved = 0;

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	    while(fileSize > currentlyRecieved)
	    {
		    memset(buffer, 0, packetSize);
        	n = read(newsockfd,buffer,packetSize-1);
            currentlyRecieved += n;
	    }

        printf("Message delivered\n");

	    close(newsockfd);
	    close(sockfd);
}
