#include "parser.h"
#include <stdio.h>
#include <string.h>
#define PIPE_BUF 4096

extern char* token[PIPE_BUF];
extern char* req[PIPE_BUF];

void  parseComArg(int argc,char* argv[]){
	
	for(int i=1;i<argc;i++){
		token[i-1]=argv[i];
	}
}

void  parse(char buf[100]){
	char s[2]=" ";

	int i=0;
	token[0]=strtok(buf,s);

	while(token[i]!=NULL){
	    i++;
	    token[i]=strtok(NULL, s);
	}
}

void parseRequest(char buf[PIPE_BUF]){
    char s[2]="\n";

    int i=0;
    req[0]=strtok(buf,s);

    while(req[i]!=NULL){
       // printf(" %s\n",token[i]);
        i++;
        req[i]=strtok(NULL, s);
    }
}