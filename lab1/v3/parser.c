#include "parser.h"
#include <stdio.h>
#include <string.h>
#define PIPE_BUF 4096

extern char* token[PIPE_BUF];
extern char* req[PIPE_BUF];

// parses command line input from client
// delimiter is eithrr '\n' or ' '
void  parse(char buf[PIPE_BUF], char delimiter[2]){
	// delimiter
	int i=0;
	// for separating according to '\n'
	if(delimiter[0]=='\n'){
		
    	req[0]=strtok(buf,delimiter);

    	while(req[i]!=NULL){
	        i++;
	        req[i]=strtok(NULL, delimiter);
    	}
    	return;
	}
	else{
		// to separate accoridng to ' '
		token[0]=strtok(buf,delimiter);

		while(token[i]!=NULL){
		    i++;
		    token[i]=strtok(NULL, delimiter);
		}
	}
}