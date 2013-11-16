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
	LSP lsp;
	LSP templsp; 
	LSP checklsp;
	
	r = router_init(*routerID,argv[3]);
	LSP_init(&r);
	
	
	
	char *log_filename;
	FILE *log_file;
	
	log_filename = argv[2];
	printf("40\n");	

	log_file = fopen(log_filename, "wb");
    if (!log_file)
    {
        printf("Failed open log file %s for router %s.\n", argv[2], argv[1]);
        exit(1);
    }
	
	r = initSock(r);
	print_router(r);
	
	int i;
	int highSock;
	char* buffer = malloc(sizeof(LSP));
	bzero(buffer,sizeof(buffer));
	
	memcpy(buffer,(char*) &lsp, sizeof(LSP));
	memcpy((char*) &checklsp, buffer, sizeof(LSP));
	
	printf("sizeof: %d\n",(int)(sizeof(r.self_packet)));
	print_LSP(&r.self_packet, log_file);
	
	fd_set readSet;
	fd_set tempReadSet; 
	
	FD_ZERO(&readSet);
	FD_ZERO(&tempReadSet);

	
	//first try to connect to neighbors 
	for(i = 0; i< r.nbrs_count; i++) 
	{
		bzero(buffer, sizeof(lsp));
		memcpy(buffer, (char *) &lsp, sizeof(lsp));
		if(TCPconnect(r.nbrs[i]) < 0)
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
					if ((newSock = TCPaccept(r.nbrs[i])) > -1)
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
					// receiving LSP
					bzero(buffer,LSPSIZE);
					int size = sizeof(&r.nbrs[i].localAddr);
					
					if(recvfrom(r.nbrs[i].localSock, buffer, LSPSIZE, 0, (struct sockaddr *) &r.nbrs[i].localAddr, &size) <0)
					{
						printf("error receiving\n");
					}
					
					//Forwarding LSP on all ports except the port it was received on 
					else
					{	
						memcpy((char *)&templsp, buffer, LSPSIZE);
						printf("file received from %s through %s\n",templsp.routerID,r.nbrs[i].ID);
						print_LSP(&templsp, log_file);
						
						int j; 
						
						for(j = 0; j< r.nbrs_count ; j++)
						{
							
							if((j != i) && (r.nbrs[j].connectedR == 1)) 
							{
								
								if(sendto(r.nbrs[j].remoteSock, buffer, LSPSIZE, 0, (struct sockaddr*)&r.nbrs[i].remoteAddr, sizeof(r.nbrs[j].remoteAddr)) < 0)
									printf("send failed sending\n");
		
								printf("from %s to %s\n", r.nbrs[i].ID, r.nbrs[j].ID);							
							
							
							}
						}
						
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
				if(TCPconnect(r.nbrs[i]) < 0)
				{
					printf("failed to connect to %s \n", r.nbrs[i].ID); 
					r.nbrs[i].connectedR = 0;
				}
				else
					r.nbrs[i].connectedR = 1;
					
			}
		}
		
		lsp.seq_num++;
		if( counter == 0 || (counter%5 == 0))
		{
		for(i = 0; i<r.nbrs_count; i++)
		{
			if(r.nbrs[i].connectedR == 1) 
			{
				buffer = malloc(LSPSIZE);
				bzero(buffer, LSPSIZE);
				memcpy(buffer, (char*) &r.self_packet, LSPSIZE);
				memcpy((char *)&templsp, buffer, LSPSIZE); 
		
				if(sendto(r.nbrs[i].remoteSock, buffer, LSPSIZE, 0, (struct sockaddr*)&r.nbrs[i].remoteAddr, sizeof(r.nbrs[i].remoteAddr)) < 0)
					printf("send failed sending\n");
		
				printf("sent LSP to %s\n", r.nbrs[i].ID);
			}
			
			
		}
		}
		counter ++;
		
	}	
	
	return 0; 
	
// end if
}
