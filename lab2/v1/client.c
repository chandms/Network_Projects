#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include<signal.h>

#define BUF_SIZE 1000

union
{
    uint32_t integer;
    unsigned char byte[4];
} conv;

union
{
    uint32_t integer;
    unsigned char byte[1];
} com;

void term_prog (int sig) {
	printf("didn't recieve the response from server\n");
    kill (0,SIGTERM);
}

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
    int lc=0,pc=0;
    time_t time1,time2;

    int sock_descriptor;
	char server_message[BUF_SIZE];
	struct sockaddr_in server_addr;
	socklen_t server_struct_length;
	int port;

	fp = fopen("pingparam.dat","r");
	if(fp==NULL){
		printf("%s\n","pingparam.dat file doesn't exist " );
		exit(EXIT_FAILURE);
	}




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
    

	

	while ((read = getline(&line, &len, fp)) != -1) {
		
        token[0]=strtok(line,delim);
        lc=0;
        while(lc<4){
        	lc++;
        	token[lc]=strtok(NULL,delim);
        	if(token[lc]==NULL)
        		break;
        }

     

    

	    int n=atoi(token[0]);
	    int t=atoi(token[1]);
	    int d=atoi(token[2]);
	    int s=atoi(token[3]);

	    printf("Read: N =%d, T= %d, D= %d, S= %d\n",n,t,d,s );
	    
	    for(lc=0; lc<n; lc++){


	    	// Converting int to byte 
	    	conv.integer = s;
			com.integer = d;
			unsigned char client_message[5];
			for(pc=0;pc<4;pc++)
				client_message[pc]=conv.byte[pc];

			client_message[pc]=com.byte[0];


			
	    	memset(server_message, '\0', sizeof(server_message));
	    	

	    	
	    	time1 = time(NULL);
	    	printf("time1 %ld\n", time1);
	    	// Send the message to server:
	    	// 
	    	
		    if(sendto(sock_descriptor, client_message, sizeof(client_message), 0,
		         (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		        printf("Unable to send message\n");
		        return -1;
		    }
		    printf("Client sent %d,%d\n", s,d);

		    if(lc+1==n){
		    	signal(SIGALRM, term_prog);
		    	alarm(10);
		    }

		    if(recvfrom(sock_descriptor, server_message, sizeof(server_message), 0,
		         (struct sockaddr*)&server_addr, &server_struct_length) < 0){
		        printf("Error while receiving server's message\n");
		        return -1;
		    }

		    if(lc+1==n){
		    	// cancelling the previous alarm if obtained the output
		    	alarm(0);
		    }

		    s++;
		    time2 = time(NULL);
	    	printf("time2 %ld\n", time2);

	    	int diff = (time2-time1)*1000;



		    printf("Received from server : %s\n",server_message );
		    printf("################## RTT = %d\n", diff);
		    
		    if(lc+1<n)
		    {
		    	printf("sleeping for %d seconds before sending the next request\n",t );
		    	printf("*******************\n\n\n");
		    	sleep(t);
		    }
	    }



    }
    

    


	

    
    fclose(fp);
    close(sock_descriptor);





    return 0;






}