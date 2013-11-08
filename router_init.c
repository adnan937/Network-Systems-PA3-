#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "router_init.h"

#define BUFSIZE 20

void direct_nbrs_init( char routerID,char *filename){
	
	FILE *f;
	
	if( (f = fopen(filename, "r")) == NULL )
		printf("unable to open the file");
		
	router_info r;
	r.id = routerID;

	printf("your shit id is= %s\n", &r.id);
	
	char line[BUFSIZE];
	int length;
	while( fgets(line,BUFSIZE,f) != NULL )
	{
		char *linePtr = line;
		linePtr++;
		length = strlen(linePtr);//Get length of string
		linePtr[length - 2] = '\0';//Set next to last char to null
		
		printf("\nline = %s\n", linePtr);
		
		char substr[10];	
		char *cptr;			// Pointer to string returned by tokenizers
		int n;
		
		cptr = strtok(linePtr,",");
		n = 1;
		while(cptr != NULL)
		{
			printf("substr = %s\n", cptr);
			cptr = strtok(NULL, ","); // Get next word
			n++; // Increment counter
		}
	
	/*
	r.nbr_id;
	r.tcp_send_port;
	r.tcp_rec_port;
	*/
		}
}

int main(int argc, char *argv[])
{
	printf("%s\n",argv[1]);
	char temp [sizeof(argv[1])];
	strcpy(temp, argv[1]);
	printf("%s\n", temp);
	direct_nbrs_init(*temp,argv[2]);
	return 0;
}
