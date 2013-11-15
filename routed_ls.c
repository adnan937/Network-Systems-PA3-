/* routed_LS.c */
/* This is a link state routing program that uses TCP protocol */



#include "router_init.h"





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
	lsp.seq_num = 0; 
	
	
	print_router(r);
	
	int i;
	int highSock;
	char* buffer = malloc(sizeof(lsp));
	bzero(buffer,sizeof(buffer));
	
	memcpy(buffer,(char*) &lsp, sizeof(lsp));
	memcpy((char*) &checklsp, buffer, sizeof(checklsp));
	printf("router id: %c\nsequence number: %d\n sizeof: %d\n",checklsp.router.ID,checklsp.seq_num, sizeof(lsp));
	
	
	fd_set readSet;
	fd_set tempReadSet; 
	
	FD_ZERO(&readSet);
	FD_ZERO(&tempReadSet);

	int sock[r.nbrs_count];
	
	
	for(i = 0; i< r.nbrs_count; i++) 
	{
		bzero(buffer, sizeof(lsp));
		if(TCPconnect(r.nbrs[i].tcp_send_port, buffer) < 0)
			r.nbrs[i].connected = 0;
		else 
			r.nbrs[i].connected = 1;

		
	} 
	
	for(i = 0; i<  r.nbrs_count; i++) 
	{
		sock[i] = TCPlisten(r.nbrs[i].tcp_rec_port);
		FD_SET(sock[i],&readSet);
		
	}
	
	highSock = sock[0]; 
	
	for(i = 0; i < r.nbrs_count - 1; i++ )
	{
		highSock = sock[i] > sock[i+1]? sock[i]: sock[i+1]; 
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
		
		
		//printf("85\n");
		for(i = 0; i< r.nbrs_count; i++) 
		{
			if(FD_ISSET(sock[i], &tempReadSet))
			{
				bzero(buffer,sizeof(lsp));
				if( TCPaccept(sock[i], buffer) == 0) 
				{
					r.nbrs[i].connected = 1;
					printf("file received from %c \n",r.nbrs[i].ID);
					memcpy((char *)&templsp, buffer, sizeof(templsp));
			
					printf("received lsp info...\n");
					printf("router id: %c\nsequence number: %d\n",templsp.router.ID,templsp.seq_num);
			
				}
				
			}
		}
		
		
		//printf("116\n");
	
		sleep(5);
		lsp.seq_num = 1 + lsp.seq_num;
			
		for(i = 0; i< r.nbrs_count; i++) 
		{	
			buffer = malloc(sizeof(lsp));
			bzero(buffer, sizeof(lsp));
			memcpy(buffer, (char*) &lsp, sizeof(lsp)); 
			
			if(r.nbrs[i].connected == 1)
			{
				if(TCPconnect(r.nbrs[i].tcp_send_port, buffer) < 0)
				{
					printf("failed to connect to %c \n", r.nbrs[i].ID); 
					r.nbrs[i].connected = 0;
				}
				else
					printf("file sent to %c \n", r.nbrs[i].ID);
			}
		}
		counter ++;
		
	}	
	
	return 0; 
	
// end if
}
