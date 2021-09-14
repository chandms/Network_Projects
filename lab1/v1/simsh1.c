// Simple shell example using fork() and execlp().

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

char* token[100];
void  parse(char buf[100]){
	char s[2]=" ";

	int i=0;
	token[0]=strtok(buf,s);

	while(token[i]!=NULL){
	   // printf(" %s\n",token[i]);
	    i++;
	    token[i]=strtok(NULL, s);
	}
	
	//printf("done\n");
}
int main(void)
{
pid_t k;
char buf[100];
int status;
int len;

  while(1) {

	//printf("hiiiii in ");
	// print prompt
  	fprintf(stdout,"[%d]$ ",getpid());

	// read command from stdin
	fgets(buf, 100, stdin);
	len = strlen(buf);
	if(len == 1) 				// only return key pressed
	  continue;
	buf[len-1] = '\0';
	parse(buf);
  	k = fork();
  	if (k==0) {
  	// child code
    	  if(execvp(buf,token) == -1)	// if execution failed, terminate child
	  	exit(1);
  	}
  	else {
  	// parent code 
	  waitpid(k, &status, 0);
  	}
  }
}
