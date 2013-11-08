/* This is a supporting file that builds a router, and initializes it
 * using the given initialization text */

	/* Node or Router structure */
	typedef struct{
		char id;			/* ID of the src router */
		char nbr_id;		/* ID of the destination router */
		int link_cost;		/* Link cost */
		int tcp_send_port;	/* TCP send port number */
		int tcp_rec_port;	/* TCP receive port number */
		struct router *next;
	}router;
	
	/* Link State Packet structure */
	typedef struct{
		char router_id;		/* ID of the src node that created LSP */
		router *nbrs;		/* List of neighbors and link cost */
		int seq_num;		/* sequence number of LSP */
		int length;			/* length of LSP */
		int ttl;			/* Time-to-Live for LSP */
	}LSP;


/* Configuration File - Initialization of Directly Connected Routers */
void direct_nbrs_init(char router_ID,char *filename);
