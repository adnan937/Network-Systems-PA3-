/* routed_LS.c */
/* This is a link state routing program that uses TCP protocol */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <time.h>

#include "router_init.h"


int main(int argc, char *argv[])
{
	/* check command line args. */
	if(argc < 4){
		printf("usage : %s <RouterID> <LogFileName> <Initialization file> \n", argv[0]);
		exit(1);
	}
	
	char *router_id;
	char *log_filename;
	char *init_file;
	FILE *log_file;
	
	// Extract arguments
	router_id = argv[1];
	log_filename = argv[2];
	init_file = argv[3];

    Router router;
    router = router_init(*router_id,init_file);
    //print_router(router);
    
    
    
    // Open log file
    log_file = fopen(log_filename, "wb");
    if (!log_file)
    {
        printf("Failed open log file %s for router %s.\n", argv[2], argv[1]);
        exit(1);
    }
    
    /*
    // ??    
    fd_set readSet;
    FD_ZERO(&readSet);
    int send_socket[router.nbrs_count];
    int recv_socket[router.nbrs_count];
    
    // Create sockets for sending/receiving to/from all direct neighbors
    int i;    
    Neighbor n;  
    for(i=0 ; i<router.nbrs_count ; i++) 
    {
		n = router.nbrs[i];
		//socket[i] = TCPlisten(n.recv_port);
		send_socket[i] = createTCPsocket(n.send_port);
		recv_socket[i] = createTCPsocket(n.recv_port);
		FD_SET(socket[i],&readSet);
	}
	
	
    
    
    int high_socket;    
    high_socket = recv_socket[0];
    for(i=0; i< router.nbrs_count-1 ; i++)
    {
		high_socket = recv_socket[i] > recv_socket[i+1]? recv_socket[i]: recv_socket[i+1]; 
    }
        
    for(i=0 ; i<router.nbrs_count ; i++) 
    {
		// if connection fails, listen!
		if(TCPconnect(router.nbrs[i].send_port) < 0)
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
    
    // Select: while router is listening, it waits for one or more neighbors
    // to connect.            
    for(;;)
    {
		printf("in for\n");                
		if(select(highSock+1, &readSet, NULL, NULL, NULL) == -1)
		{
			perror("Server-select() error lol!");
			exit(1);
        }
        
        for(i=0; i<router.nbrs_count ; i++) 
        {
			if(FD_ISSET(sock[i], &readSet))
            {
				TCPaccept(sock[i]);        
            }
        }
        
    }*/
    
     // Set router timer
    time_t curr_time;
    time(&router.timestamp);
    
    // Initialize LSP of this router
    LSP_init(&router);
	print_LSP(&router.self_packet,log_file);
	
	// Each router has an array of recently received packets
	LSP received_packets[5];
	// Receive buffer
    LSP buffer_packet;
    
    // initialize router routing table
    routing_table_init(&router);
    print_routing_table(&router, log_file);
    
    // some connection shit
    /*
     // check time, send self lsp on all ports with established link
     time(&curr_time);
     if (difftime(curr_time, router.timestamp) >= (double)5.0)
     {
		 router.timestamp = curr_time;
		 // update lsp seq
		 router.self_packet.seq++;
		 for (i=0 ; i<router.nbrs_count ; i++)
		 {
                if (router.nbrs[i].connected)
                {
                    nbytes = send(socket?, router.self_pakcet, sizeof(LSP), 0);
                    if (nbytes == -1)
                    {
                        printf("Failed to send on link: %s, %d, %s, %d\n",
                        router.nbrs[i].src_ID, router.nbrs[i].send_port,
                        router.nbrs[i].ID, router.nbrs[i].recv_port);
                    }
                    else
                    {
                        printf("Send LSP to: %s\n", router.nbrs[i].ID);
                    }
                }
            }
        }
        
     int lsp_update_flag;  
     // Receive LSP from all ports with "connected" neighbors
     for (i=0 ; i<router.nbrs_count ; i++)
     {
		 if (router.nbrs[i].connected)
		 {
			 nbytes = recv(router.nbrs[i].connect_fd, &buffer_packet, sizeof(LSP), 0);
			 if (nbytes > 0)
			 {
				 printf("LSP received from ID %s, seq %d\n", buffer_packet.router.ID, buffer_lsp.seq_num);
				 // store recvd lsp into database and determine if need forwarding
				 // also determine if need update topology and recompute
				 lsp_update_flag = update_LSP_list(&router, &buffer_lsp);
				 if (lsp_update_flag == 1)
				 {
					 // 1. Run Dijkstra!
					 printf("Update routing table...\n");
					 if (update_routing_table(&(router), &buffer_packet))
					 {
						 time(&curr_time);
                         sprintf(tmp_char_buffer, "UTC:\t%s", asctime(gmtime(&curr_time)));
                         printf("%s",tmp_char_buffer);
                         fwrite(tmp_char_buffer, sizeof(char), strlen(tmp_char_buffer), log_file);
                         print_lsp(&buffer_packet, log_file);
                         log_routing_table(&router, log_file);
                     }
					 
					 // 2. Flood LSP to all outgoing links, except where it came from
                     buffer_pacekt.ttl--;
                     int j;
                     for (j=0; j<router.nbrs_count; j++)
                     {
						 if ((j != i) && (router.nbrs[j].connected))
                         {
							 nbytes = send(router.nbrs[j].connect_fd, &buffer_packet, sizeof(LSP), 0);
							 if (nbytes == -1)
                             {
								printf("Failed to send on link: %s, %d, %s, %d\n",
								router.nbrs[i].src_ID, router.nbrs[i].send_port,
								router.nbrs[i].ID, router.nbrs[i].recv_port);
                             }
                             else
                             {
                                 printf("Forward LSP from %s to %s\n", buffer_packet.routerID, router.nbrs[j].ID);
                             }
                         }
                     }
                 }
                 /*
                 if(lsp_update_flag == 2)
                 {
					 // run dijkstra's algorithm
					 printf("Update routing table...\n");
					 if (update_routing_table(&(router), &buffer_packet))
					 {
						 time(&curr_time);
                         sprintf(tmp_char_buffer, "UTC:\t%s", asctime(gmtime(&curr_time)));
                         printf("%s",tmp_char_buffer);
                         fwrite(tmp_char_buffer, sizeof(char), strlen(tmp_char_buffer), log_file);
                         print_lsp(&buffer_packet, log_file);
                         log_routing_table(&router, log_file);
                     }
                 }*/
             //}
         //}

     //}
                
        return 0; 
        
// end if
	
}
