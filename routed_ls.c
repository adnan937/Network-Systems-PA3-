/* routed_LS.c */
/* This is a link state routing program that uses TCP protocol */


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

void TCPlisten(int port)
{
		int servSock = createTCPserverSocket(port); 
	
		if( listen(servSock, QUEUELENGTH) < 0)
			printf("listening error \n");	
			
		printf("listening...\n");
		for(;;)
		{
			
			struct sockaddr_in cliAddr; ;
			int clientSock;
			int cliLen = sizeof(cliAddr);
		
			if((clientSock = accept(servSock, (struct sockaddr *) &cliAddr, &cliLen)) < 0) 
				printf("server accepting failed\n"); 
				
			char buffer[200]; 
			bzero(buffer,sizeof(buffer));
			
			if( recvfrom(clientSock, &buffer, sizeof(buffer), 0, (struct sockaddr *) &cliAddr, &cliLen) == -1)
				printf(" error with getting the file\n"); 
				
			printf("file received %s\n", buffer); 	
			
			//close(clientSock);
		}
	
		 
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
	printf("file sent!\n");
		
	return 0;
}

int main(int argc, char *argv[]) 
{
	/* check command line args. */
	if(argc < 4){
		printf("usage : %s <RouterID> <LogFileName> <Initialization file> \n", argv[0]);
		exit(1);
	}
	
	
	printf("...\n"); 
	if( TCPconnect(9602) < 0)
	{
		printf("connecting failed!\n");
		TCPlisten(9601);
	}
	
	return 0; 
}
