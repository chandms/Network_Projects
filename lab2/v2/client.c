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

#define MAX 100
#define SA struct sockaddr

int main(int argc, char* argv[]){

	if(argc<3){
		printf("%s\n","please provide all the inputs" );
		return 0;
	}

	int sock_desc, connfd;
    struct sockaddr_in servaddr, cliaddr;
   
    // socket create and varification
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));
   
    // connect the client socket to server socket
    if (connect(sock_desc, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
   
   
    char buff[MAX];
    while(1){
    	bzero(buff, sizeof(buff));
    	printf("Enter the command  : ");
    	fgets(buff, MAX, stdin);
        buff[strlen(buff)-1]='\0';
        write(sock_desc, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        read(sock_desc, buff, sizeof(buff));
        printf("From Server : %s", buff);
    }
   
    // close the socket
    close(sock_desc);
}