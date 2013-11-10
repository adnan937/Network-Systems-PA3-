/* This is a supporting file that builds a router, and initializes it
 * using the given initialization text */

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
Router direct_nbrs_init(char router_ID,char *filename);

/*
 * prints router ID, number of neighbors and neighbors with their IDs,
 * link costs, and TCP's send and receive port numbers.
 */
void print_router(Router router);

void log_file(char *logfile, char *msg);
