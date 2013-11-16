#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* This is a supporting file that builds a router, and initializes it
 * using the given initialization text */

#define MAX_NEIGHBORS 5
#define MAX_NODES 6
#define BUFSIZE 20
#define LSP_TTL 10
#define LSPSIZE 56
#define INFINITY 999
#define QUEUELENGTH 4    
		
	/* Neighbor node structure */
	typedef struct{
		char src_ID[1];				/* ID of the parent router */
		char ID[1];					/* ID of the destination router */
		int send_port;				/* TCP send port number */
		int recv_port;				/* TCP receive port number */
		int link_cost;				/* Link cost */
		int connectedS, connectedR;	/* Connection Flag */
		int localSock, remoteSock;	/* Sockets FDs */
		struct sockaddr_in localAddr, remoteAddr;
		int node_num;				/* convert router name to node number */
	}Neighbor;
	
	typedef struct{
		char dest[1];
		int link_cost;
		int node_num;
		}LSPentry;
	
	/* Link State Packet structure */
	typedef struct{
		char routerID[1];				/* src node that created the LSP */
		int seq_num;					/* sequence number of LSP */
		LSPentry table[MAX_NEIGHBORS];	/* Array list of neighbors and costs */
		int length;						/* how many neighbors src router has */
		int ttl;						/* Time-to-Live for LSP */
	}LSP;
	
	
	/* Node or Router structure */
	typedef struct{
		char ID[1];						/* ID of the src router */
		Neighbor nbrs[MAX_NEIGHBORS];	/* Array list of neighbors */
		int nbrs_count;					/* How many neighbors do we actually have */
		LSP self_packet;
		LSP recved_packets[MAX_NODES];
		time_t timestamp;				/*  */
		int routing_table[MAX_NODES][MAX_NODES];
		int node_num;
	}Router;
		
	
/* 
 * Configuration File - Initialization of Directly Connected Routers 
 * return: a Router structure that is read from filename, with its 
 * neighbors, TCP send and receive ports, and link cost.
 */
Router router_init(char router_ID,char *filename)
{
	
	FILE *f;
	char line[BUFSIZE];
	char *linePtr;
	int length;
	char *substr;		// Pointer to subString returned by tokenizer
	int n;				// Counter for the number of chars
	
	// declare and initialize a router
	Router router;
	//router.ID = router_ID;
	strcpy(router.ID, &router_ID);
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
		if(substr[0] == router_ID)
		{
			Neighbor nbr;
			
			// While the line isn't finished
			while(substr != NULL)		
			{
				switch(n)
				{
					case 1:
						strcpy(nbr.src_ID,substr);
						break;
					case 2:
						nbr.send_port = atoi(substr);
						break;
					case 3:
					{
						strcpy(nbr.ID,substr);
						if(*nbr.ID == 'A')
							nbr.node_num = 0;
						if(*nbr.ID == 'B')
							nbr.node_num = 1;
						if(*nbr.ID == 'C')
							nbr.node_num = 2;
						if(*nbr.ID == 'D')
							nbr.node_num = 3;
						if(*nbr.ID =='E')
							nbr.node_num = 4;
						if(*nbr.ID =='F')
							nbr.node_num = 5;
						break;
					}
					case 4:
						nbr.recv_port = atoi(substr);
						break;
					case 5:
						nbr.link_cost = atoi(substr);
						break;
				}
				substr = strtok(NULL, ","); // Get next word
				n++;					    // Increment counter for words in the line
			}
			router.nbrs[router.nbrs_count] = nbr;
			router.nbrs_count++;
		}

	}
	
	// Convert router names to a node number
	if(*router.ID == 'A')
		router.node_num = 0;
	if(*router.ID == 'B')
		router.node_num = 1;
    if(*router.ID == 'C')
		router.node_num = 2;
    if(*router.ID == 'D')
		router.node_num = 3;
    if(*router.ID =='E')
		router.node_num = 4;
    if(*router.ID =='F')
		router.node_num = 5;
	
	fclose(f);

	return router;
}


/*
 * prints router ID, number of neighbors and neighbors with their IDs,
 * link costs, and TCP's send and receive port numbers.
 */
void print_router(Router router)
{
    printf("Router ID: %s, num of neighbors %d\n", router.ID, router.nbrs_count);
    int i;
    Neighbor n;
    printf("Dest.\tCost\tOut_TCP_port\tDest_TCP_port\n");
    for (i=0; i<router.nbrs_count; i++)
    {
        n = router.nbrs[i];
        printf("%s\t%d\t\t%d\t\t%d\n", n.ID, n.link_cost, n.send_port,n.recv_port);
    }
}

/*
 * Initialize LSP of this router
 */
void LSP_init(Router *router)
{
    strcpy(router->self_packet.routerID, router->ID);
    router->self_packet.seq_num = 0;
	router->self_packet.length = router->nbrs_count;
	
	int i;
	Neighbor n;
	for(i=0 ; i<router->self_packet.length ; i++)
	{
		n = router->nbrs[i];
		strcpy(router->self_packet.table[i].dest, n.ID);
		router->self_packet.table[i].link_cost = n.link_cost;
		if(*n.ID == 'A')
			router->self_packet.table[i].node_num = 0;
		if(*n.ID == 'B')
			router->self_packet.table[i].node_num = 1;
		if(*n.ID == 'C')
			router->self_packet.table[i].node_num = 2;
		if(*n.ID == 'D')
			router->self_packet.table[i].node_num = 3;
		if(*n.ID =='E')
			router->self_packet.table[i].node_num = 4;
		if(*n.ID =='F')
			router->self_packet.table[i].node_num = 5;
	}
		
	router->self_packet.ttl = LSP_TTL;
}

/*
 * 
 */
void print_LSP(LSP *lsp, char *log_filename)
{
	int i;
    char str[2048];
    
    // Open log file
    FILE * file;
    file = fopen(log_filename, "r+");
    if (!file)
    {
        printf("Failed open log file\n");
        exit(1);
    }
    
    sprintf(str, "Link State Packet of %s------------\nDest\tCost\n", lsp->routerID);
    if (file) 
    {
		fwrite(str, sizeof(char), strlen(str), file);
		printf("%s", str);
	}

     for (i=0 ; i<lsp->length ; i++)
    {
        sprintf(str, "%s\t%d\n", lsp->table[i].dest, lsp->table[i].link_cost);
        if (file) 
		{
			fwrite(str, sizeof(char), strlen(str), file);
			printf("%s", str);
		}
	}
	
	fclose(file);
}

/* 
 * Update a router packet's list with the received packet
 * return:  0 discard this LSP.
 * 			1 LSP is new! LSP_list is updated, and needs forwarding.
 * 			2 LSP is new! LSP_list is updated, LSP causes change to the table, and needs forwarding.
 */
int check_LSP(Router *router, LSP *lsp)
{
	int tmp;
	int i,j;
	char nodes [6];
    nodes[0] = 'A';
    nodes[1] = 'B';
    nodes[2] = 'C';
    nodes[3] = 'D';
    nodes[4] = 'E';
    nodes[5] = 'F';
		
    // check TTL, discard lsp if TTL = 0
    if (lsp->ttl <=0)
    {
		tmp = 0;
		return tmp;
	}
	
	LSPentry *entry;
	for(i=0 ; i<6 ; i++)
	{
		if(*lsp->routerID == nodes[i])
		{
			// Check if packet is new!
			if(router->recved_packets[i].seq_num < lsp->seq_num)
			{
				// update lsp list.
				router->recved_packets[i] = *lsp;
				tmp = 1;
				
				for(j=0 ; j<lsp->length ; j++)
				{
					entry = lsp->table;
					if(router->routing_table[i][entry->node_num] != entry->link_cost)
					{	
						tmp= 2;
						// update table.
						router->routing_table[i][entry->node_num] = entry->link_cost;
					}
				}
			}
		}
	}
	return tmp;
}

/* 
 * Initializing routing table with direct links costs, and INFINITY for
 * everything else.
 */
 
void routing_table_init(Router *router)
{
    int row;
    int col;
    Neighbor *nbr;

    for (row=0 ; row<MAX_NODES ; row++)
   {
	   for(col=0 ; col<MAX_NODES ; col++)
	   {
		   router->routing_table[row][col] = INFINITY;
	   }
   }
    
   for (col=0 ; col<router->nbrs_count ; col++)
   {
	   nbr = &(router->nbrs[col]);
	   router->routing_table[router->node_num][nbr->node_num] = nbr->link_cost;
   }
}   

/*
 * 
 */
void print_routing_table(Router *router, char* log_filename)
{   
    int i,j;
    char str[1024];
    
    char nodes [6];
    nodes[0] = 'A';
    nodes[1] = 'B';
    nodes[2] = 'C';
    nodes[3] = 'D';
    nodes[4] = 'E';
    nodes[5] = 'F';
    
    // Open log file
    FILE * file;
    file = fopen(log_filename, "r+");
    if (!file)
    {
        printf("Failed open log file \n");
        exit(1);
    }
    
    sprintf(str, "Routing Table of %s------------\nA\tB\tC\tD\tE\tF\n", router->ID);
    if (file)
    {
        fwrite(str, sizeof(char), strlen(str), file);
        printf("%s", str);
    }
    
    for (i=0 ; i<MAX_NODES ; i++)
    {	
		printf("%c\t", nodes[i]);
		for(j=0 ; j<MAX_NODES ; j++)
		{
			sprintf(str, "%d\t",router->routing_table[i][j]);
			if (file)
			{
				fwrite(str, sizeof(char), strlen(str), file);
				printf("%s", str);
			}
		}
		sprintf(str, "\n");
		if (file)
		{
			fwrite(str, sizeof(char), strlen(str), file);
			printf("%s", str);
		}
	}
	fclose(file);
}

int allselected(int *selected)
{
	int i;
	for(i=0;i<MAX_NODES;i++)
		if(selected[i] == 0)
			return 0;
    return 1;
}

/* 
 * compute Dijkstra's shortest path tree
 * return:  1 for updated routing table
 *          0 for routing table unchanged
 
 
int update_routing_table(Router *router, int *cost)
{
//void shortpath(int cost[][MAX], unsigned int *nextHop, unsigned int *distance)

		router->routing_table;
        
        int selected[MAX_NODES] = {0};
        int current=0,i,k,n,dc,p,q,smalldist,newdist;
        
        for(i=0;i<MAX;i++)
                distance[i]=INFINITE;
                router->table
        
        selected[current]=1;
        distance[0]=0;
        current=0;
        n=0;
        /*
        for(p=0;p<MAX;p++)
        {
                printf("For router %d  \n", p);
                for(q=0;q<MAX;q++)
                {
                        printf("Shortpath recieved %d  \n", cost[p][q]);
                        
                }
                                
        }
        
        while(!allselected(selected))
        {
                
                
                smalldist=INFINITE;
                dc=distance[current];
                for(i=0;i<MAX;i++)
                {
                        if(selected[i]==0)
                        {
                                newdist=dc+cost[current][i];
                                if(newdist<distance[i])
                                {
                                        distance[i]=newdist;
                                        nextHop[i]=current;
                                        printf(" nextHop[i] is :  %d \n", nextHop[i]);
                                }
                                if(distance[i]<smalldist)
                                {
                                        smalldist=distance[i];
                                        printf(" NewSmallest route:  %d \n", smalldist);

                                        k=i;
                                }
                        }
                }
                current=k;
                selected[current]=1;
                n++;
                //if(n>=7)
                        //break;
                
        }
}*/

/* */
Router initSock(Router r) 
{
        int i; 
        
        for(i =0;i < r.nbrs_count; i++)
        {
                
                if((r.nbrs[i].localSock = socket( PF_INET,SOCK_STREAM , IPPROTO_TCP)) < 0) 
                        printf("error creating server socket! \n"); 
                
                r.nbrs[i].localAddr.sin_family = AF_INET;
                r.nbrs[i].localAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
                r.nbrs[i].localAddr.sin_port = htons(r.nbrs[i].send_port);
                
                r.nbrs[i].connectedS = 0;
                
                if((r.nbrs[i].remoteSock = socket( PF_INET,SOCK_STREAM , IPPROTO_TCP)) < 0) 
                        printf("error creating client socket! \n"); 
                
                r.nbrs[i].remoteAddr.sin_family = AF_INET;
                r.nbrs[i].remoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //serverIP
                r.nbrs[i].remoteAddr.sin_port = htons(r.nbrs[i].recv_port);
                
                r.nbrs[i].connectedR = 0;
        }
        
        
        return r; 
}

/* */
void TCPlisten(Neighbor nbr)
{
        
        if(bind(nbr.localSock,(struct sockaddr *) &nbr.localAddr, sizeof(nbr.localAddr)) < 0)
                printf("binding failed\n");
        
        if( listen(nbr.localSock, QUEUELENGTH) < 0)
                        printf("listening error \n");
        
}

/* */
int TCPaccept(Neighbor nbr)
{
        
        socklen_t size = sizeof(nbr.localAddr);
        int newSock;
        if((newSock = accept(nbr.localSock,(struct sockaddr *) &nbr.localAddr, &size))< 0)
        {                
                printf("accpeting failed\n");   
                return -1; 
        }
        printf("Neighbor %s has been accpeted\n", nbr.ID);
        return newSock;
}

/* */
int TCPconnect(Neighbor nbr)
{
        printf("trying to connect...\n");
              
        if(connect(nbr.remoteSock, (struct sockaddr *) &nbr.remoteAddr, sizeof(nbr.remoteAddr)) < 0)
        {
                printf("TCPconnect failed connecting\n");
                return -1; 
        }
        printf("connected to neighbor %s\n", nbr.ID);
    return 0;            
}
