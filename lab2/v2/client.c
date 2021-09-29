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
   

    // connect the client socket to server socket
    if (connect(sock_desc, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
}

void establish_connection_with_server(){
	close_socket();
	initiate_socket();
	connect_to_socket();
}


void term_prog (int sig) {
	printf("didn't recieve the response from server\n");
	if(count_timer==0)
	{
		printf("Terminating Client Request Process \n");
    	kill (0,SIGTERM);
	}
    else{
    	
    	printf("counter timer = %d\n",count_timer );
    	count_timer--;
    	close_socket();


    	
    }


}



int main(int argc, char* argv[]){
	count_timer=3;
	flag = false;

	


	if(argc<3){
		printf("%s\n","please provide all the inputs" );
		return 0;
	}

	ipc[0] = argv[1];
	prt[0] = argv[2];

	printf("Enter the command  : ");
    bzero(buff, sizeof(buff));
    fgets(buff, MAX, stdin);
    buff[strlen(buff)-1]='\0';


	while(1){
		printf("Client Connection Starts\n");
		initiate_socket();
	    connect_to_socket();

		//establish_connection_with_server();
	   
	    // getting input from client
	   	
	    

		bzero(output, sizeof(output));
	    //signal(SIGALRM, term_prog);
	   
	    write(sock_desc, buff, sizeof(buff));
	    printf("writing done \n");

	    struct sigaction action;
  		action.sa_handler = term_prog;
  		action.sa_flags = SA_RESTART; //<-- restart 
  		sigaction(SIGALRM, &action, NULL);
	    alarm(2);

	    read(sock_desc, output, sizeof(output));
	    
	    close_socket();
	    if(strcmp(output,"")!=0)
	    {
	    	printf("hi From Server : %s\n", output);
	    	kill(0,SIGTERM);
	    }

	    printf("********************************\n\n\n");
	    //kill (0,SIGTERM);
	}
}