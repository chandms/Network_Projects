#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include<signal.h>
#define MAX 4096
#define SA struct sockaddr

char* token[100];

int security_check(struct sockaddr_in cliaddr, char buf[MAX]){

	char* ip = inet_ntoa(cliaddr.sin_addr);

	char* tk[100];
	char dm[2]=".";

	int i=0;
	tk[0]=strtok(ip,dm);
	while(tk[i]!=NULL){
		i++;
		tk[i]=strtok(NULL,dm);
	}

	if(!(strcmp(tk[0],"127")==0 && strcmp(tk[1],"0")==0 && (strcmp(tk[2],"0")==0 || strcmp(tk[2],"1")==0)))
	{
		printf("ip address invalid \n");
		return -1;
	}

	printf("IP address check successful \n");

	if(strcmp(buf,"date")==0 || strcmp(buf,"/bin/date")==0){
		printf("message is secure  \n");
		return 1;
	}

	else
	{
		printf("Message is unsecured\n");
		return -1;
	}

	return -1;

}
void  parse(char buf[100]){

	int k=0;
	for(k=0;k<100;k++)
		token[k]=NULL;
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
	token[i]=NULL;
}

int main(int argc, char *argv[]){

	if(argc<3){
		printf("%s\n","please provide all the inputs");
		return 0;
	}

	int port = atoi(argv[2]);

	int socket_desc, connfd;
	socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buff[MAX];
    
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   

    printf("port = %d, ip = %s\n",port,argv[1]);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(port);


    if ((bind(socket_desc, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    
    if ((listen(socket_desc, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");


    while(1){

	    bzero(&cliaddr, sizeof(cliaddr));
	    len = sizeof(cliaddr);
	   	printf("\n\n*****************\n");
	    
	    connfd = accept(socket_desc, (SA*)&cliaddr, &len);
	    if (connfd < 0) {
	        printf("server accept failed...\n");
	        exit(0);
	    }
	    else
	        printf("server accepted the client...\n");
	   
	    
	    time_t t;
	    srand((unsigned) time(&t));
	    int toss_val = rand()%2;
	    //int toss_val =0;
	    printf("value of toss is = %d\n",toss_val);

	    // head comes
	    if(toss_val==0){
	    	printf("Head came up, ignoring this request \n");
	    	continue;
	    }
	    else{
	    	printf("Tail came, processing the request \n");


	    	bzero(buff, MAX);
   
        	// read the message from client and copy it in buffer
        	read(connfd, buff, sizeof(buff));

	    	// tails come
	    	printf("Received message from IP: %s and port: %i\n",
	           inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

	    	printf("Client Message : %s\n", buff);

	    	int res = security_check(cliaddr,buff);

	    	if(res==1){
	    	

	    		printf("Security check is successful \n");
		    	parse(buff);

		    	pid_t frk;
		    	int status;
		    	
		    	frk = fork();
		    	
		    	
		    	if(frk==0){
		    		dup2(connfd,1);
		    		if(execvp(token[0],token) == -1)
		    		{
		    			fprintf(stderr, "%s\n", "server can't process the request");
		                exit(1);
		    		}

		    	}
		    	else{
		    		waitpid(frk, &status, 0);
		    		printf("child exited \n");
		    	}
	    	}
	    	else{
	    		printf("Server can't process unsecured request\n");
	    	}
	    }
	}
   
    close(socket_desc);
}


