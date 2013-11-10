#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router_init.h"

#define BUFSIZE 20


Router direct_nbrs_init(char router_ID,char *filename)
{
	
	FILE *f;
	char line[BUFSIZE];
	char *linePtr;
	int length;
	char *substr;		// Pointer to subString returned by tokenizer
	int n;				// Counter for the number of chars
	
	// declare and initialize a router
	Router router;
	router.ID = router_ID;
	printf("Here's your shit ID:%c \n", router.ID);
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
		//printf("substr = %s\n", substr);
		//printf("routerID= %c\n", router_ID);
		if(substr[0] == router_ID)
		{
			Neighbor nbr;
			
			// While the line isn't finished
			while(substr != NULL)		
			{
				switch(n)
				{
					case 1:
						nbr.src_ID = substr[0];
						//printf("nbr.src_id=%c \n",nbr.src_ID);
						break;
					case 2:
						nbr.tcp_send_port = atoi(substr);
						//printf("nbr.tcp_send_port=%d \n",nbr.tcp_send_port);
						break;
					case 3:
						nbr.ID = substr[0];
						//printf("nbr_id=%c \n",nbr.ID);
						break;
					case 4:
						nbr.tcp_rec_port = atoi(substr);
						//printf("nbr.tc_rec_port=%d \n",nbr.tcp_rec_port);
						break;
					case 5:
						nbr.link_cost = atoi(substr);
						//printf("nbr.link_cost=%d \n",nbr.link_cost);
						break;
				}
				substr = strtok(NULL, ","); // Get next word
				n++;					    // Increment counter for words in the line
			}
			router.nbrs[router.nbrs_count] = nbr;
			router.nbrs_count++;
		}

	}
	fclose(f);
	return router;
}


void print_router(Router router)
{
    printf("Router ID: %c, num of neighbors %d\n", router.ID, router.nbrs_count);
    int i;
    Neighbor n;
    printf("Dest.\tCost\tOut_TCP_port\tDest_TCP_port\n");
    for (i=0; i<router.nbrs_count; i++)
    {
        n = router.nbrs[i];
        printf("%c\t%d\t\t%d\t\t%d\n", n.ID, n.link_cost, n.tcp_send_port,n.tcp_rec_port);
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


int main(int argc, char *argv[])
{
	char routerID [sizeof(argv[1])];
	strcpy(routerID, argv[1]);
	Router r;
	r = direct_nbrs_init(*routerID,argv[2]);
	
	print_router(r);
	
	return 0;
}

