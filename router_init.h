/* This is a supporting file that builds a router, and initializes it
 * using the given initialization text */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>   
#include <sys/time.h> 
#include <signal.h>
#include <unistd.h>

#define QUEUELENGTH 4 	
#define MAX_NEIGHBORS 5

	/* */
	typedef struct{
		char src_ID;			/* ID of the parent router */
		int tcp_send_port;		/* TCP send port number */
		char ID;				/* ID of the destination router */
		int tcp_rec_port;		/* TCP receive port number */
		int link_cost;			/* Link cost */
	}Neighbor;
	
	/* Node or Router structure */
	typedef struct{
		char ID;						/* ID of the src router */
		Neighbor nbrs[MAX_NEIGHBORS];	/* Array list of neighbors */
		int nbrs_count;					/* How many neighbors do we actually have */
	}Router;
	
	/* Link State Packet structure */
	typedef struct{
		Router router;		/* src node that created the LSP */
		int seq_num;		/* sequence number of LSP */
		int length;			/* length of LSP */
		int ttl;			/* Time-to-Live for LSP */
	}LSP;


/* 
 * Configuration File - Initialization of Directly Connected Routers 
 * Reads the filename line-by-line, and if the source router in the read
 * line matches the passed routerID, it saves the data into a Router
 * structure. Otherwise, discards it. 
 */
//Router direct_nbrs_init(char router_ID,char *filename);

/*
 * prints router ID, number of neighbors and neighbors with their IDs,
 * link costs, and TCP's send and receive port numbers.
 */
//void print_router(Router router);

//void log_file(char *logfile, char *msg);

int createTCPserverSocket(int portNumber)
{
	// could be something other than AF_INET
	int socketId;
	if((socketId = socket( PF_INET,SOCK_STREAM , IPPROTO_TCP)) < 0) 
		printf("error creating server socket! \n"); 
	
	struct sockaddr_in servAddr; 
	
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servAddr.sin_port = htons(portNumber);
	
	
	if(bind(socketId,(struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		printf("binding failed\n");
	
	
	
		
	return socketId;
}

int TCPlisten(int port)
{
	int servSock = createTCPserverSocket(port); 
	
	if( listen(servSock, 1) < 0)
			printf("listening error \n");	
		
	printf("listening. port: %d ...\n",port);
	
	//close(clientSock);
		
	return servSock;	 
}

void TCPaccept(int sock)
{
	
	struct sockaddr_in cliAddr; ;
	int clientSock;
	int cliLen = sizeof(cliAddr);
			
			
									
	if((clientSock = accept(sock, (struct sockaddr *) &cliAddr, &cliLen)) < 0) 
		printf("server accepting failed\n"); 
				
	char buffer[200]; 
	bzero(buffer,sizeof(buffer));
			
	if( recvfrom(clientSock, &buffer, sizeof(buffer), 0, (struct sockaddr *) &cliAddr, &cliLen) == -1)
		printf(" error with getting the file\n"); 
				
	printf("%s\n", buffer); 
	
}

int TCPconnect(int portNumber)
{ 	
	
	printf("trying to connect...\n");
	int status; 

	int socketId; 
	if((socketId = socket( PF_INET,SOCK_STREAM , IPPROTO_TCP)) < 0) 
		printf("error creating client socket! \n"); 
	
	
	struct sockaddr_in remoteAddr; 
	
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //serverIP
	remoteAddr.sin_port = htons(portNumber);

	
	if( connect(socketId, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr)) < 0) 
		return -1; 
	
	char buffer[] = " hello, world"; 
	//bzero(buffer,sizeof(buffer));
	
	
	if(sendto(socketId, buffer, sizeof(buffer), 0,(struct sockaddr *)&remoteAddr, sizeof(remoteAddr))==-1)
		printf("unable to send!\n");
	else
		{
			printf("file sent!\n");
		
			for(;;)
			{
			}
		}	
	return 0;
}

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "router_init.h"

#define BUFSIZE 20


Router direct_nbrs_init(char router_ID,char *filename)
{
	
	FILE *f;
	char line[BUFSIZE];
	char *linePtr;
	int length;
	char *substr;		// Pointer to subString returned by tokenizer
	int n;				// Counter for the number of chars
	
	// declare and initialize a router
	Router router;
	router.ID = router_ID;
	printf("Here's your shit ID:%c \n", router.ID);
	router.nbrs_count = 0;
	
	if( (f = fopen(filename, "r")) == NULL )
		printf("unable to open the file");
	
	while( fgets(line,BUFSIZE,f) != NULL )
	{
		linePtr = line;
		linePtr++;					//Get rid of first char
		length = strlen(linePtr);	//Get length of string
		linePtr[length - 2] = '\0';	//Set next to last char to null
		
		//printf("\nline = %s\n", linePtr);
		
		// Read first char of the line "Src router"
		substr = strtok(linePtr,",");
		n = 1;
		//printf("substr = %s\n", substr);
		//printf("routerID= %c\n", router_ID);
		if(substr[0] == router_ID)
		{
			Neighbor nbr;
			
			// While the line isn't finished
			while(substr != NULL)		
			{
				switch(n)
				{
					case 1:
						nbr.src_ID = substr[0];
						//printf("nbr.src_id=%c \n",nbr.src_ID);
						break;
					case 2:
						nbr.tcp_send_port = atoi(substr);
						//printf("nbr.tcp_send_port=%d \n",nbr.tcp_send_port);
						break;
					case 3:
						nbr.ID = substr[0];
						//printf("nbr_id=%c \n",nbr.ID);
						break;
					case 4:
						nbr.tcp_rec_port = atoi(substr);
						//printf("nbr.tc_rec_port=%d \n",nbr.tcp_rec_port);
						break;
					case 5:
						nbr.link_cost = atoi(substr);
						//printf("nbr.link_cost=%d \n",nbr.link_cost);
						break;
				}
				substr = strtok(NULL, ","); // Get next word
				n++;					    // Increment counter for words in the line
			}
			router.nbrs[router.nbrs_count] = nbr;
			router.nbrs_count++;
		}

	}
	fclose(f);
	return router;
}


void print_router(Router router)
{
    printf("Router ID: %c, num of neighbors %d\n", router.ID, router.nbrs_count);
    int i;
    Neighbor n;
    printf("Dest.\tCost\tOut_TCP_port\tDest_TCP_port\n");
    for (i=0; i<router.nbrs_count; i++)
    {
        n = router.nbrs[i];
        printf("%c\t%d\t\t%d\t\t%d\n", n.ID, n.link_cost, n.tcp_send_port,n.tcp_rec_port);
    }
}

int flag=0;
void log_file(char *logfile, char *msg)
{
	FILE *f;
	//f = fopen(logfile, "r");
	
	// logfile is not created
	if(flag == 0)
	{
		if( (f = fopen(logfile, "w")) == NULL)
			printf("unable to create logfile\n");
			flag = 1;
	}
	
	// Appened new msgs to already existed logfile.
	else
	{
		if( (f = fopen(logfile, "a")) == NULL)
			printf("unable to append to logfile\n");
			
		else
			fputs(msg, f);
	}
	
	fclose(f);
}
