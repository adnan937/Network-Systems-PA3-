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
	
	int nbytes;
	
	// Extract arguments
	router_id = argv[1];
	log_filename = argv[2];
	init_file = argv[3];

    Router router;
    router = router_init(*router_id,init_file);
    //printf("Router init:\n");
    //print_router(router);
    
    
    
    // Open log file
    log_file = fopen(log_filename, "wb");
    if (!log_file)
    {
        printf("Failed open log file %s for router %s.\n", argv[2], argv[1]);
        exit(1);
    }
    
    router = initSock(router);
    //printf("Router after initSock:\n");
    //print_router(router);
    
    // Initialize LSP of this router
    LSP_init(&router);
	print_LSP(&router.self_packet,log_filename);
	
	LSP lsp = router.self_packet;
	// Each router has an array of recently received packets
	// Receive buffer
    //LSP buffer_packet;
    LSP templsp; 
    //LSP checklsp;
    
    char* buffer = malloc(sizeof(LSP));
    //bzero(buffer,sizeof(buffer));
    //memcpy(buffer,(char*) &lsp, sizeof(LSP));
    //memcpy((char*) &checklsp, buffer, sizeof(LSP));
    //print_LSP(&checklsp, log_file);
    //printf("router id: %c\nsequence number: %d\n sizeof: %d\n",checklsp.router.ID,checklsp.seq_num, (int) sizeof(lsp));
    
    // initialize router routing table
    routing_table_init(&router);
    print_routing_table(&router, log_filename);
    
    fclose(log_file);
    // ??    
        int i, j;
    fd_set readSet;
    fd_set tempReadSet; 
    
    FD_ZERO(&readSet);
    FD_ZERO(&tempReadSet);
    
    //first try to connect to neighbors 
    for(i = 0; i< router.nbrs_count; i++) 
    {
		if(TCPconnect(router.nbrs[i]) < 0)
			router.nbrs[i].connectedR = 0;
		else 
			router.nbrs[i].connectedR = 1;
     } 
	
	
     //then listen 
     for(i = 0; i<  router.nbrs_count; i++) 
     {
		 TCPlisten(router.nbrs[i]);
		 FD_SET(router.nbrs[i].localSock,&readSet);
      }
    
    int highSock;

    highSock = router.nbrs[0].localSock; 
    for(i = 0; i < router.nbrs_count-1; i++ )
    {
		highSock = router.nbrs[i].localSock > router.nbrs[i+1].localSock? router.nbrs[i].localSock: router.nbrs[i+1].localSock; 
    }
    
    int counter = 0; 
    struct timeval tv; 
    tv.tv_sec = 5;         
    tv.tv_usec = 0; 
    
    // Select: while router is listening, it waits for one or more neighbors
    // to connect.            
    for(;;)
    {
		tv.tv_sec = 5;         
        tv.tv_usec = 0;
                        
        tempReadSet = readSet; 
        printf("- %d\n", counter);
                                
        if(select(highSock+1, &tempReadSet, 0, 0, &tv) < 0) 
        {
			printf("select error!\n");
		}
		
		//CHECK FD
		for(i = 0; i< router.nbrs_count; i++) 
		{
			// for select: Check if there's a connection or not
            if(FD_ISSET(router.nbrs[i].localSock, &tempReadSet))
            {
				// Accept connections
				if( router.nbrs[i].connectedS == 0) 
				{
					int newSock;
					if ((newSock = TCPaccept(router.nbrs[i])) > -1)
					{
						router.nbrs[i].connectedS= 1;
						router.nbrs[i].localSock = newSock;
						
						printf("sock: %d\n",router.nbrs[i].localSock);
						printf("connectedS: %d\n",router.nbrs[i].connectedS);
						
						FD_SET(router.nbrs[i].localSock,&readSet);
						if(highSock < newSock)
							highSock = newSock; 
					}
				}
				
				// Receive LSPs
				else
				{
					bzero(buffer,sizeof(LSP));
					socklen_t size = sizeof(&router.nbrs[i].localAddr);
					
					if(recvfrom(router.nbrs[i].localSock, buffer, sizeof(LSP), 0, (struct sockaddr *) &router.nbrs[i].localAddr, &size) <0)
					{
						printf("error receiving from node %s\n", router.nbrs[i].ID);
					}
					else
					{
						printf("LSP received from %s \n",router.nbrs[i].ID);
						memcpy((char *)&templsp, buffer, sizeof(LSP));
						//print_LSP(&templsp,log_file);
						
						// Check if received lsp is new
						int check_lsp_flag = check_LSP(&router, &templsp);
						if (check_lsp_flag == 1 || check_lsp_flag == 2)
						{
							// Log if LSP caused change in routing table.
							if(check_lsp_flag == 2)
							{
								print_LSP(&templsp, log_filename);
								print_routing_table(&router,log_filename);
							}
							
							// 2. Flood to all links, except where it came from
							for(j = 0; j<router.nbrs_count; j++)
							{
								if((j != i) && router.nbrs[i].connectedR == 1)
								{
									//buffer = malloc(sizeof(LSP));
									//bzero(buffer, sizeof(LSP));
									//memcpy(buffer, (char*) &lsp, sizeof(LSP));
									//memcpy((char *)&templsp, buffer, sizeof(LSP)); 
									
									nbytes = sendto(router.nbrs[j].remoteSock, buffer, LSPSIZE, MSG_NOSIGNAL, (struct sockaddr*)&router.nbrs[j].remoteAddr, sizeof(router.nbrs[j].remoteAddr));
									if (nbytes == -1)
									{
										printf("Failed to send on link: %s, %d, %s, %d\n",
										router.nbrs[i].src_ID, router.nbrs[i].send_port,
										router.nbrs[i].ID, router.nbrs[i].recv_port);
									}
									
									else
									{
										printf("Forward LSP from %s to %s\n", templsp.routerID, router.nbrs[j].ID);
										//print_LSP(&templsp, log_file);
									}
								}
													
													
							}
																			 
						}
					}
                                
                                
                }
            }
		}
                
            sleep(5);
                
            // Try to connect again.        
            for(i = 0; i< router.nbrs_count; i++) 
            {
				if(router.nbrs[i].connectedS == 1 &&router.nbrs[i].connectedR == 0)
				{
					if(TCPconnect(router.nbrs[i]) < 0)
					{
						printf("failed to connect to %s \n", router.nbrs[i].ID); 
						router.nbrs[i].connectedR = 0;
					}
					else
						router.nbrs[i].connectedR = 1;
				}
			}
			
			// initial and periodic flooding: Send LSP to connected links.
			lsp.seq_num++;
			for(i = 0; i<router.nbrs_count; i++)
			{
				if(router.nbrs[i].connectedR == 1)
				{
					buffer = malloc(sizeof(LSP));
					bzero(buffer, sizeof(LSP));
					memcpy(buffer, (char*) &lsp, sizeof(LSP));
					memcpy((char *)&templsp, buffer, sizeof(LSP)); 
					
					nbytes = sendto(router.nbrs[i].remoteSock, buffer, sizeof(LSP), 0, (struct sockaddr*)&router.nbrs[i].remoteAddr, sizeof(router.nbrs[i].remoteAddr));
					if (nbytes == -1)
					{
						printf("Failed to send on link: %s, %d, %s, %d\n",
						router.nbrs[i].src_ID, router.nbrs[i].send_port,
						router.nbrs[i].ID, router.nbrs[i].recv_port);
					}
					
					else
					{
						printf("Send LSP to: %s\n", router.nbrs[i].ID);
						//print_LSP(&templsp, log_file);
					}
				}
			}
			
			counter ++;  
        
        
	}
	fclose(log_file);
	return 0;      
    
     // Set router timer
    //time_t curr_time;
    //time(&router.timestamp);
    
    
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
         
        
// end if
	
}
