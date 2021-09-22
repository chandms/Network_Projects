#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define BUF_SIZE 1000

int main(int argc, char* argv[]){

	if(argc<4){
		printf("%s\n","please provide all the inputs" );
		return 0;
	}

	FILE *fp;
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char delim[2]=",";
    char* token[4];
    int lc=0;
    time_t time1,time2;

	fp = fopen("pingparam.dat","r");
	if(fp==NULL){
		printf("%s\n","pingparam.dat file doesn't exist " );
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
        
        printf("%s\n", line);
        token[0]=strtok(line,delim);
        lc=0;
        while(lc<4){
        	lc++;
        	token[lc]=strtok(NULL,delim);
        	if(token[lc]==NULL)
        		break;
        }

        break;

    }

    printf("%s\n", "done");

    fclose(fp);

    int n=atoi(token[0]);
    int t=atoi(token[1]);
    int d=atoi(token[2]);
    int s=atoi(token[3]);

    printf("%d,%d,%d,%d\n",n,t,d,s );
    int sock_descriptor;
	char server_message[BUF_SIZE];
	struct sockaddr_in server_addr;
	socklen_t server_struct_length;
	int port;

	port = atoi(argv[3]);
	server_struct_length = sizeof(server_addr);

	if ( (sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed in client");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[2]);
    char client_message[1000];
    for(lc=0; lc<n; lc++){

    	memset(server_message, '\0', sizeof(server_message));
    	memset(client_message, '\0', sizeof(client_message));
    	
    	char mid[1000];
    	char command[100];
    	sprintf(mid, "%d", s);
    	strcpy(client_message,mid);
    	sprintf(command,"%d",d);
    	strcat(client_message," ");
    	strcat(client_message,command);

    	
    	time1 = time(NULL);
    	printf("time1 %ld\n", time1);
    	// Send the message to server:
    	// 
    	printf("client_message %s\n",client_message );
	    if(sendto(sock_descriptor, client_message, strlen(client_message), 0,
	         (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
	        printf("Unable to send message\n");
	        return -1;
	    }

	    printf("%s\n", "client sent message");

	    if(recvfrom(sock_descriptor, server_message, sizeof(server_message), 0,
	         (struct sockaddr*)&server_addr, &server_struct_length) < 0){
	        printf("Error while receiving server's msg\n");
	        return -1;
	    }

	    s++;
	    time2 = time(NULL);
    	printf("time2 %ld\n", time2);

    	int diff = (time2-time1)*1000;



	    printf("Received from server : %s\n",server_message );
	    printf("sleeping for %d seconds\n",t );

	    printf("\n\n RTT = %d\n", diff);
	    printf("*******************\n\n\n");
	    sleep(t);
    }
    

    


	

    

    close(sock_descriptor);





    return 0;






}