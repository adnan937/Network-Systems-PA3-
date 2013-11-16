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

#define LSPSIZE 56
#define QUEUELENGTH 4 	
#define MAX_NEIGHBORS 5
#define MAX_NODES 6
#define LSP_TTL 10
	/* */
	
	typedef struct{
                int cost[MAX_NODES][MAX_NODES];
    }RoutingTable;
        
     typedef struct{
                char dest[1];
                int link_cost;
    }LSPentry;   
    
	typedef struct{
		char src_ID[1];
		int connectedS, connectedR; 
		int tcp_send_port;		/* TCP send port number */
		char ID[1];				/* ID of the destination router */
		int tcp_rec_port;		/* TCP receive port number */
		int link_cost;			/* Link cost */
		struct sockaddr_in localAddr, remoteAddr; 
		int localSock, remoteSock; 
		int node_num;
	}Neighbor;
	
	/* Link State Packet structure */
	typedef struct{
		LSPentry table[MAX_NEIGHBORS];
		char routerID[1];   /* src node that created the LSP */
		int seq_num;		/* sequence number of LSP */
		int length;			/* length of LSP */
		int ttl;			/* Time-to-Live for LSP */
	}LSP;
	
	
	typedef struct{
		char ID[1];                                                /* ID of the src router */
        Neighbor nbrs[MAX_NEIGHBORS];        /* Array list of neighbors */
        int nbrs_count;                                        /* How many neighbors do we actually have */
        LSP self_packet;
        LSP recved_packets[MAX_NODES];
        time_t timestamp;                                /*  */
        RoutingTable routing_table;
        int node_num;
	}Router;

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

Router initSock(Router r) 
{
	int i; 
	
	for(i =0;i < r.nbrs_count; i++)
	{
		
		if((r.nbrs[i].localSock = socket( PF_INET,SOCK_STREAM , IPPROTO_TCP)) < 0) 
			printf("error creating server socket! \n"); 
		
		r.nbrs[i].localAddr.sin_family = AF_INET;
		r.nbrs[i].localAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
		r.nbrs[i].localAddr.sin_port = htons(r.nbrs[i].tcp_send_port);
		
		r.nbrs[i].connectedS = 0;
		
		if((r.nbrs[i].remoteSock = socket( PF_INET,SOCK_STREAM , IPPROTO_TCP)) < 0) 
			printf("error creating client socket! \n"); 
		
		r.nbrs[i].remoteAddr.sin_family = AF_INET;
		r.nbrs[i].remoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //serverIP
		r.nbrs[i].remoteAddr.sin_port = htons(r.nbrs[i].tcp_rec_port);
		
		r.nbrs[i].connectedR = 0;
	}
	
	
	return r; 
}

void TCPlisten(Neighbor nbr)
{
	
	if(bind(nbr.localSock,(struct sockaddr *) &nbr.localAddr, sizeof(nbr.localAddr)) < 0)
		printf("binding failed\n");
	
	if( listen(nbr.localSock, QUEUELENGTH) < 0)
			printf("listening error \n");
	
}

int TCPaccept(Neighbor nbr)
{
	
	int size = sizeof(nbr.localAddr);
	int newSock;
	if((newSock = accept(nbr.localSock,(struct sockaddr *) &nbr.localAddr,&size))< 0)
	{		
		printf("accpeting failed\n");   
		return -1; 
	}
	
	printf("Neighbor %s has been accpeted\n", nbr.ID);
	return newSock;
}

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


#define BUFSIZE 20


Router router_init(char router_ID,char *filename)
{
        
        FILE *f;
        char line[BUFSIZE];
        char *linePtr;
        int length;
        char *substr;                // Pointer to subString returned by tokenizer
        int n;                                // Counter for the number of chars
        
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
                linePtr++;                                        //Get rid of first char
                length = strlen(linePtr);        //Get length of string
                linePtr[length - 2] = '\0';        //Set next to last char to null
                
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
                                                nbr.tcp_send_port = atoi(substr);
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
                                                nbr.tcp_rec_port = atoi(substr);
                                                break;
                                        case 5:
                                                nbr.link_cost = atoi(substr);
                                                break;
                                }
                                substr = strtok(NULL, ","); // Get next word
                                n++;                                            // Increment counter for words in the line
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



void print_router(Router router)
{
    printf("Router ID: %s, num of neighbors %d\n", router.ID, router.nbrs_count);
    int i;
    Neighbor n;
    printf("Dest.\tCost\tOut_TCP_port\tDest_TCP_port\n");
    for (i=0; i<router.nbrs_count; i++)
    {
        n = router.nbrs[i];
        printf("%s\t%d\t\t%d\t\t%d\n", n.ID, n.link_cost, n.tcp_send_port,n.tcp_rec_port);
    }
}
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
