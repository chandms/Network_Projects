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
#include <stdbool.h>

#define MAX 4096
#define SA struct sockaddr
void send_receive_request();
void term_prog (int sig);

int count_timer;


char buff[MAX];
char output[MAX];
char* prt[1];
char* ipc[1];
bool flag;
int sock_desc, connfd;
struct sockaddr_in servaddr, cliaddr;
struct sigaction sact;

void initiate_socket(){
	 sock_desc = socket(AF_INET, SOCK_STREAM, 0);
	    if (sock_desc == -1) {
	        printf("socket creation failed...\n");
	        exit(0);
	    }
	    else
	        printf("Socket successfully created..\n");
}

void close_socket(){
	close(sock_desc);
}

void connect_to_socket(){
	bzero(&servaddr, sizeof(servaddr));
	   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipc[0]);
    servaddr.sin_port = htons(atoi(prt[0]));
  
    if (connect(sock_desc, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
}


void term_prog (int sig) {
	printf("\n\n\n********************************\n");
	printf("didn't recieve the response from server\n");
	if(count_timer==0)
	{
		printf("Repeated the request thrice, terminating Client Request Process \n");
		close_socket();
		// closing the existing connection on terminating the request.
	}
    else{
    	
    	printf("counter timer = %d\n",count_timer );
    	count_timer--;
    	close_socket();
    	initiate_socket();
    	connect_to_socket();

    	bzero(output, sizeof(output));
	    write(sock_desc, buff, sizeof(buff));
	    printf("writing done \n");

    	signal(SIGALRM, term_prog);
    	alarm(2);


    	
    }


}



int main(int argc, char* argv[]){
	
	flag = false;

	


	if(argc<3){
		printf("%s\n","please provide all the inputs" );
		return 0;
	}

	ipc[0] = argv[1];
	prt[0] = argv[2];

	// getting input from client
	while(1){

		count_timer=3;
		printf("Enter the command  : ");
	    bzero(buff, sizeof(buff));
	    fgets(buff, MAX, stdin);
	    buff[strlen(buff)-1]='\0';


		
		printf("Client Connection Starts\n");
		initiate_socket();
	    connect_to_socket();
	   
	   
	   	
	    

		bzero(output, sizeof(output));
	    write(sock_desc, buff, sizeof(buff));
	    printf("writing done \n");

	    signal(SIGALRM, term_prog);
	    alarm(2);
	   
	    read(sock_desc, output, sizeof(output));
	    close_socket();
	    if(strcmp(output,"")!=0)
	    {
	    	// cancelling the alarm after recieving response
	    	alarm(0);
	    	printf("From Server : %s\n", output);
	    	
	    }
	}

}