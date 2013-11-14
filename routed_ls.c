


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
	
	print_router(r);
	
	int i;
	int highSock;
	
	fd_set readSet;
	FD_ZERO(&readSet);
	int sock[r.nbrs_count];
	
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
	
	for(i = 0; i< r.nbrs_count; i++) 
	{
		
		if(TCPconnect(r.nbrs[i].tcp_send_port) < 0)
			printf("failed to connect"); 
	} 
	
	
	//int i;
	 
	//sock[0] = TCPlisten(8080);
	//sock[1] = TCPlisten(8081);
	 
	
	// add ports to the list
	//FD_SET(sock[0], &readSet); 
	//FD_SET(sock[1], &readSet); 
	
	
	//if (sock[0] > sock[1]) 
		//highSock = sock[0]; 
	//else
		//highSock = sock[1]; 
		
	for(;;)
	{
		printf("in for\n");		
		if(select(highSock+1, &readSet, NULL, NULL, NULL) == -1)
		{
			perror("Server-select() error lol!");
			exit(1);
		}
		
		for(i = 0; i< r.nbrs_count; i++) 
		{
			if(FD_ISSET(sock[i], &readSet))
			{
				TCPaccept(sock[i]);
			
			}
		}
		
		
		
		
	}	
		
	return 0; 
	
// end if
}
