#include "parser.h"
#include "parser.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>

// shared variable
extern char* token[100];


void  parseComArg(int argc,char* argv[]){
	
	for(int i=1;i<argc;i++){
		token[i-1]=argv[i];
	}
}

// defining parse 
void  parse(char buf[100]){
	// delimiter
	char s[2]=" ";

	int i=0;
	// getting the first token
	token[0]=strtok(buf,s);

	while(token[i]!=NULL){
		// going through other tokens
	    i++;
	    token[i]=strtok(NULL, s);
	}
}