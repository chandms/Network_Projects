#include "parser.h"
#include <stdio.h>
#include <string.h>
#define PIPE_BUF 4096

// shared among parser, server.
extern char* token[PIPE_BUF];

// parse function definition
void  parse(char buf[PIPE_BUF]){
	// delimiter
	char s[2]=" ";

	int i=0;
	// generating the first token
	token[0]=strtok(buf,s);

	while(token[i]!=NULL){
		// generating other tokens
	    i++;
	    token[i]=strtok(NULL, s);

	}
}