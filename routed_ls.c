/* routed_LS.c */
/* This is a link state routing program that uses TCP protocol */



#include "router_init.h"

#define LSPSIZE 362




int main(int argc, char *argv[]) 
{
	/* check command line args. */

	if(argc < 2){
		printf("usage : %s <RouterID> <LogFileName> <Initialization file> \n", argv[0]);
		exit(1);
	}
	
	
	char routerID [sizeof(argv[1])];
	strcpy(routerID, argv[1]);
	Router r;
	r = direct_nbrs_init(*routerID,argv[2]);
	LSP lsp;
	LSP templsp; 
	LSP checklsp;
	lsp.router = r; 
	lsp.seq_num = 1; 
	
	r = initSock(r);
	print_router(r);
	
	int i;
	int highSock;
	char* buffer = malloc(sizeof(LSPSIZE));
	bzero(buffer,sizeof(buffer));
	
	memcpy(buffer,(char*) &lsp, LSPSIZE);
	memcpy((char*) &checklsp, buffer, LSPSIZE);
	printf("router id: %c\nsequence number: %d\n sizeof: %d\n",checklsp.router.ID,checklsp.seq_num, (int) sizeof(lsp));
	
	
	fd_set readSet;
	fd_set tempReadSet; 
	
	FD_ZERO(&readSet);
	FD_ZERO(&tempReadSet);

	int sock[r.nbrs_count];
	
	//first try to connect to neighbors 
	for(i = 0; i< r.nbrs_count; i++) 
	{
		bzero(buffer, sizeof(lsp));
		memcpy(buffer, (char *) &lsp, sizeof(lsp));
		if(TCPconnect(r.nbrs[i], buffer) < 0)
			r.nbrs[i].connectedR = 0;
		else 
			r.nbrs[i].connectedR = 1;
	} 
	
	//then listen 
	for(i = 0; i<  r.nbrs_count; i++) 
	{
		TCPlisten(r.nbrs[i]);
		FD_SET(r.nbrs[i].localSock,&readSet);
		
	}
	
	highSock = r.nbrs[0].localSock; 
	
	for(i = 0; i < r.nbrs_count-1; i++ )
	{
		highSock = r.nbrs[i].localSock > r.nbrs[i+1].localSock? r.nbrs[i].localSock: r.nbrs[i+1].localSock; 
	}
	
	int counter = 0; 
	struct timeval tv; 
	tv.tv_sec = 5; 	
	tv.tv_usec = 0;
		
	for(;;)
	{	
		tv.tv_sec = 5; 	
		tv.tv_usec = 0;
			
		tempReadSet = readSet; 
		printf("- %d\n", counter);
				
		if(select(highSock+1, &tempReadSet, 0, 0, &tv) < 0) 
		{
			if("select error!\n");
		}
		
		
		for(i = 0; i< r.nbrs_count; i++) 
		{
			
			if(FD_ISSET(r.nbrs[i].localSock, &tempReadSet))
			{	
				
				if( r.nbrs[i].connectedS == 0) 
				{	
					int newSock;
					if ((newSock = TCPaccept(r.nbrs[i], buffer)) > -1)
					{
						r.nbrs[i].connectedS= 1;
						r.nbrs[i].localSock = newSock;
						
						printf("sock: %d\n",r.nbrs[i].localSock);
						printf("connectedS: %d\n",r.nbrs[i].connectedS);
						
						FD_SET(r.nbrs[i].localSock,&readSet);
						
						if(highSock < newSock)
							highSock = newSock; 
					}
				}
				
				else
				{	
					
					bzero(buffer,LSPSIZE);
					int size = sizeof(&r.nbrs[i].localAddr);
					
					if(recvfrom(r.nbrs[i].localSock, buffer, LSPSIZE, 0, (struct sockaddr *) &r.nbrs[i].localAddr, &size) <0)
					{
						printf("error receiving\n");
					}
					else
					{	
						printf("file received from %c \n",r.nbrs[i].ID);
						memcpy((char *)&templsp, buffer, LSPSIZE);
						printf("router id: %c\nsequence number: %d\n",templsp.router.ID,templsp.seq_num);
					}
				}
				
				
			}
		}
		
		
	
		sleep(5);
		
			
		for(i = 0; i< r.nbrs_count; i++) 
		{	
			
			if(r.nbrs[i].connectedS == 1 &&r.nbrs[i].connectedR == 0)
			{
				bzero(buffer,sizeof(LSPSIZE));
				memcpy(buffer,(char*)&lsp, LSPSIZE);
				if(TCPconnect(r.nbrs[i],buffer) < 0)
				{
					printf("failed to connect to %c \n", r.nbrs[i].ID); 
					r.nbrs[i].connectedR = 0;
				}
				else
					r.nbrs[i].connectedR = 1;
					
			}
		}
		
		lsp.seq_num++;
		for(i = 0; i<r.nbrs_count; i++)
		{
			if(r.nbrs[i].connectedR == 1) 
			{
				buffer = malloc(LSPSIZE);
				bzero(buffer, LSPSIZE);
				memcpy(buffer, (char*) &lsp, LSPSIZE);
				memcpy((char *)&templsp, buffer, LSPSIZE); 
		
				if(sendto(r.nbrs[i].remoteSock, buffer, LSPSIZE, 0, (struct sockaddr*)&r.nbrs[i].remoteAddr, sizeof(r.nbrs[i].remoteAddr)) < 0)
					printf("send failed sending\n");
		
				printf("sent LSP number %d to %c\n",templsp.seq_num,r.nbrs[i].ID);
			}
			
			
		}
		
		counter ++;
		
	}	
	
	return 0; 
	
// end if
}
