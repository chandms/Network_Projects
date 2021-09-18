
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "parser.h"

// shared among simsh1.c, parser.c and parser.h
extern char* token[100];

int main(int argc,char* argv[])
{
pid_t k;
char buf[100];
int status;
int len;

  while(1) {

	
  fprintf(stdout,"[%d]$ ",getpid());

	// read command from stdin
	fgets(buf, 100, stdin);
	len = strlen(buf);
	if(len == 1) 				// only return key pressed
	  continue;
	buf[len-1] = '\0';

  // calling parse function from parser.h
	parse(buf);

  	k = fork();
  	if (k==0) {
  	// child code
    	  if(execvp(token[0],token) == -1)	// if execution failed, terminate child
	  	    exit(1);
  	}
  	else {
  	// parent code 
	  waitpid(k, &status, 0);
  	}

  }

  return 0;
}
