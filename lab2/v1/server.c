#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

int main(int argc, char* argv[]){
	if(argc<3){
		printf("%s\n","please provide all the inputs");
		return 0;
	}
	
	int id=0;
	int port = atoi(argv[2]);
	char server_message[1000];
	unsigned char client_message[5];
	pid_t frk;
	int status;

	int sock_descriptor;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_struct_length = sizeof(client_addr);
	socklen_t server_struct_length = sizeof(server_addr);

	if ( (sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed in server");
        exit(EXIT_FAILURE);
    }

    

    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(bind(sock_descriptor, (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }

    printf("%s\n", "binding done");

    while(1){
    	printf("\n\n************************************\n");
    	printf("server starting to connect to clients\n");

    	
    	memset(server_message, '\0', sizeof(server_message));

	    if (recvfrom(sock_descriptor, client_message, sizeof(client_message), 0,
	         (struct sockaddr*)&client_addr, &client_struct_length) < 0){
	        printf("Couldn't receive\n");
	        return -1;
	    }

	    printf("Received message from IP: %s and port: %i\n",
	           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	    // Converting byte to int 
		uint32_t myInt1 = client_message[0] + (client_message[1] << 8) + (client_message[2] << 16) + (client_message[3] << 24);
	    printf("Message from client = %d\n",myInt1 );
	    uint32_t command = client_message[4];
	    printf("Obtained command = %d\n", command);

	    sprintf(server_message, "%d", myInt1);
	    
	    if(command==0){
		    if (sendto(sock_descriptor, server_message, strlen(server_message), 0,
		         (struct sockaddr*)&client_addr, client_struct_length) < 0){
		        printf("Can't send\n");
		        return -1;
		    }
		}
		else if(command>=1 && command<=5){

			frk = fork();
			if(frk==0){
				printf("sleeping for %d seconds\n",command );
				sleep(command);
				if (sendto(sock_descriptor, server_message, strlen(server_message), 0,
			         (struct sockaddr*)&client_addr, client_struct_length) < 0){
			        printf("Can't send\n");
			        return -1;
			    }
			    printf("done sending message \n");
			}
			else{
				waitpid(frk, &status, 0);
			}
		}
		else if(command==99){
			close(sock_descriptor);
			printf("server exiting \n");
			exit(1);
		}

	}
    
    // Close the socket:
    close(sock_descriptor);
}