#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* This is a supporting file that builds a router, and initializes it
 * using the given initialization text */

#define MAX_NEIGHBORS 5
#define MAX_NODES 6
#define BUFSIZE 20
#define LSP_TTL 10
#define INFINITY 999

	/* Routing Table Structure */
	typedef struct{
		int cost[MAX_NODES][MAX_NODES];
	}RoutingTable;
		
	/* Neighbor node structure */
	typedef struct{
		char src_ID[1];		/* ID of the parent router */
		char ID[1];			/* ID of the destination router */
		int send_port;		/* TCP send port number */
		int recv_port;		/* TCP receive port number */
		int link_cost;		/* Link cost */
		int connected;
		int send_socket;
		struct sockaddr_in localAddr, remoteAddr;
		int node_num;
	}Neighbor;
	
	typedef struct{
		char dest[1];
		int link_cost;
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
		RoutingTable routing_table;
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
	}
		
	router->self_packet.ttl = LSP_TTL;
}

/*
 * 
 */
void print_LSP(LSP *lsp, FILE *file)
{
	int i;
    char str[2048];
    
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
}

/* 
 * Update a router packet's list with the received packet
 * return:  0 discard this LSP.
 * 			1 LSP updated and needs forwarding.
 */
int update_LSP_list(Router *router, LSP *lsp)
{
	int tmp;
		
    // check TTL, discard lsp if TTL = 0
    if (lsp->ttl <=0)
    {
		tmp = 0;
		return tmp;
	}
	
    if(*(lsp->routerID) == 'A')
    {
		if(router->recved_packets[0].seq_num < lsp->seq_num)
		{
			router->recved_packets[0] = *lsp;
			tmp = 1;
		}
		else tmp = 0;
	}	
	if(*(lsp->routerID) == 'B')
	{
		if(router->recved_packets[1].seq_num < lsp->seq_num)
		{
			router->recved_packets[1] = *lsp;
			tmp = 1;
		}
		else tmp = 0;
	}
	if(*(lsp->routerID) == 'C')
	{
		if(router->recved_packets[2].seq_num < lsp->seq_num)
		{
			router->recved_packets[2] = *lsp;
			tmp = 1;
		}
		else tmp = 0;
	}		
	if(*(lsp->routerID) == 'D')
	{
		if(router->recved_packets[3].seq_num < lsp->seq_num)
		{
			router->recved_packets[3] = *lsp;
			tmp = 1;
		}
		else tmp = 0;
	}	
	if(*(lsp->routerID) == 'E')
	{
		if(router->recved_packets[4].seq_num < lsp->seq_num)
		{
			router->recved_packets[4] = *lsp;
			tmp = 1;
		}
		else tmp = 0;
	}	
	if(*(lsp->routerID) == 'F')
	{
		if(router->recved_packets[5].seq_num < lsp->seq_num)
		{
			router->recved_packets[5] = *lsp;
			tmp = 1;
		}
		else tmp = 0;	
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
		   router->routing_table.cost[row][col] = INFINITY;
	   }
   }
    
   for (col=0 ; col<router->nbrs_count ; col++)
   {
	   nbr = &(router->nbrs[col]);
	   router->routing_table.cost[router->node_num][nbr->node_num] = nbr->link_cost;
   }
}   

/*
 * 
 */
void print_routing_table(Router *router, FILE* file)
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
    
    sprintf(str, "Routing Table of %s------------\n\tA\tB\tC\tD\tE\tF\n", router->ID);
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
			sprintf(str, "%d\t",router->routing_table.cost[i][j]);
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
}

/* 
 * compute Dijkstra's shortest path tree
 * return:  1 for updated routing table
 *          0 for routing table unchanged
 
int update_routing_table(Router *router, LSP *lsp)
{
    int rtn = 0;
    int i, j, k,newcost, basic_cost = MAX_COST;
    int basic_out_port, basic_dst_port;
    int exist = 0;
    RoutingTable *routingtable = &(router->routingtable);
    if (mode)
    {// add lsp mode
        // find cost between(router->ID, lsp->ID) in current routing table
        for (i=0; i<routingtable->len; i++)
        {
            if(strcmp(lsp->ID, routingtable->tableContent[i].dst)==0)
            {
                basic_cost = routingtable->tableContent[i].cost;
                basic_out_port = routingtable->tableContent[i].out_port;
                basic_dst_port = routingtable->tableContent[i].dst_port;
            }
        }

        for (i=0; i<lsp->len; i++)
        {
            exist = 0;
            for (j=0; j<routingtable->len; j++)
            {
                if(strcmp(lsp->table[i].dst, routingtable->tableContent[j].dst)==0)
                {
                    // destination already in routing table
                    exist = 1;
                    // compare cost
                    newcost = basic_cost + lsp->table[i].cost;
                    if (newcost < routingtable->tableContent[j].cost)
                    {// find path with lower cost
                        rtn = 1;
                        routingtable->tableContent[j].cost = newcost;
                        routingtable->tableContent[j].out_port = basic_out_port;
                        routingtable->tableContent[j].dst_port = basic_dst_port;
                        strcpy(routingtable->tableContent[j].nextHop, lsp->ID);
                    }
                    if (newcost == routingtable->tableContent[i].cost)
                    {// compare nextHop ID
                        if (lsp->ID[0] < routingtable->tableContent[j].nextHop[0])
                        {// find path with same cost but lower ID
                            rtn = 1;
                            routingtable->tableContent[j].cost = newcost;
                            routingtable->tableContent[j].out_port = basic_out_port;
                            routingtable->tableContent[j].dst_port = basic_dst_port;
                            strcpy(routingtable->tableContent[j].nextHop, lsp->ID);
                        }
                    }
                }
                if (strcmp(lsp->table[i].dst, router->ID)==0)
                {
                    exist = 1;
                }
            }
            if (!exist)
            {// add dst into routing table
                strcpy(routingtable->tableContent[routingtable->len].dst, lsp->table[i].dst);
                newcost = basic_cost + lsp->table[i].cost;
                routingtable->tableContent[routingtable->len].cost = newcost;
                routingtable->tableContent[routingtable->len].out_port = basic_out_port;
                routingtable->tableContent[routingtable->len].dst_port = basic_dst_port;
                strcpy(routingtable->tableContent[routingtable->len].nextHop, lsp->ID);
                routingtable->len++;
                rtn = 1;
            }
        }
    }
    else
    {// remove lsp info from routing table
    }    
    return rtn;
}   */ 

int createTCPsocket(int portNumber)
{
	// could be something other than AF_INET
    int socketId;
    if((socketId = socket( PF_INET,SOCK_STREAM , 0)) < 0) 
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
	int servSock = createTCPsocket(port); 
        
    if(listen(servSock, 1) < 0)
		printf("listening error \n");        
	
	printf("listening. port: %d ...\n",port);
	
	//close(clientSock);
    
    return servSock;         
}

void TCPaccept(int sock)
{
	struct sockaddr_in cliAddr; ;
    int clientSock;
    socklen_t cliLen = sizeof(cliAddr);
    
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
    //int status; 

    int socketId; 
    if((socketId = socket( PF_INET,SOCK_STREAM , 0)) < 0) 
		printf("error creating client socket! \n"); 
        
    struct sockaddr_in remoteAddr; 
        
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //serverIP
    remoteAddr.sin_port = htons(portNumber);

        
    if(connect(socketId, (struct sockaddr *) &remoteAddr, sizeof(remoteAddr)) < 0) 
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

