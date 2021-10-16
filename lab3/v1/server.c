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
#define CLREQ 10
#define SA struct sockaddr

// integer byte converter

union
{
    uint32_t integer;
    unsigned char byte[4];
} conv;

int ip_address_check(struct sockaddr_in cliaddr){

	char* ip = inet_ntoa(cliaddr.sin_addr);

	char* tk[100];
	char dm[2]=".";

	int i=0;
	tk[0]=strtok(ip,dm);
	while(tk[i]!=NULL){
		i++;
		tk[i]=strtok(NULL,dm);
	}

	// if(!(strcmp(tk[0],"128")==0 && strcmp(tk[1],"10")==0 && (strcmp(tk[2],"25")==0 || strcmp(tk[2],"112")==0)))
	// {
	// 	printf("ip address invalid \n");
	// 	return -1;
	// }

	if(!(strcmp(tk[0],"127")==0 && strcmp(tk[1],"0")==0 && (strcmp(tk[2],"0")==0 || strcmp(tk[2],"1")==0)))
	{
		printf("ip address invalid \n");
		return -1;
	}

	printf("IP address check successful \n");

	return 1;

}

int main(int argc, char *argv[]){

	if(argc<5){
		printf("%s\n","please provide all five command line arguments as follows - ");
		printf("%myftps server-IP server-port secret-key blocksize\n");
		printf("******************************\n");
		return 0;
	}

	int port = atoi(argv[2]);
	char* ip_address = argv[1];
	int secrect_key = atoi(argv[3]);
	int block_size = atoi(argv[4]);

	int socket_desc, connfd;
	socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    unsigned char buff[CLREQ];
    
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
   	printf("blocksize = %d\n",block_size );

    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_address);
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
    	printf("\n\n*****************\n");
	    bzero(&cliaddr, sizeof(cliaddr));
	    len = sizeof(cliaddr);
	   	
	    
	    connfd = accept(socket_desc, (SA*)&cliaddr, &len);
	    if (connfd < 0) {
	        printf("server accept failed...\n");
	        exit(0);
	    }
	    else
	        printf("server accepted the client...\n");
	   
	   	printf("Established Connection with Client of IP: %s and port: %i\n",
	           inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	    
	    bzero(buff, CLREQ);
   
        	// read the message from client and copy it in buffer
        read(connfd, buff, sizeof(buff));

        // decoding back the secret key
        int sec_key_recv = (buff[1]<<8)+buff[0];

        if(secrect_key!=sec_key_recv){
        	printf("provided secret-key = %d does not match with server's secret key %d\n",sec_key_recv,secrect_key );
        	continue;
        }

        // decoding the filename
        char fname_recv[8];
        int cn=0;
        for(int u=2;u<CLREQ;u++){
        	fname_recv[cn]=(char)buff[u];
        	cn++;
        }


        printf("Received Message From Client - %d, %s\n",sec_key_recv,fname_recv);


        // ip address check
	    int res = ip_address_check(cliaddr);
	    if(res==-1){
	    	printf("Server can't process unsecured request\n");
	    	continue;
	    }


	    time_t t;
	    srand((unsigned) time(&t));
	    int toss_val = rand()%10;
	    printf("toss = %d\n",toss_val);

	    if(toss_val>=5)
	    	toss_val=1;
	    else
	    	toss_val=0;
	    toss_val=1;
	    // head comes
	    if(toss_val==0){
	    	printf("Head came up, ignoring this request \n");
	    	continue;
	    }
	    else{
	    	printf("Tail came, processing the request \n");

		    	pid_t frk;
		    	int status;
		    	FILE *fp;
		    	fp = fopen(fname_recv,"r");
		    	if(fp==NULL){
		    		printf("%s File does not exist\n",fname_recv);
		    		continue;
		    	}
		    	
		    	// if all other checks are successful, then forking a child to process the file
		    	frk = fork();
		    	
		    	
		    	if(frk==0){


			    	while (1) {
			    		char buffer[block_size];
		    			bzero(&buffer, sizeof(buffer));
		    			// reading file
				        ssize_t read_return = fread(buffer, (size_t)1, (size_t) block_size, fp);
				        
				        
				        if (read_return == -1) {
				            perror("error while reading");
				            exit(EXIT_FAILURE);
				        }
				        
				        buffer[strlen(buffer)-1]='\0';
				        // server writes the read portion in socket
				        if (write(connfd, buffer, read_return) == -1) {
				            perror("error while writing");
				            exit(EXIT_FAILURE);
				        }

				        if (read_return <block_size){

				        	break;
				        }

				    }

				    exit(1);
		    		

		    	}
		    	else{
		    		close(connfd);
		    		printf("closed the socket\n");
		    		waitpid(frk, &status, 0);
		    		printf("child exited \n");
		    	}
	    	
	    }
	}
   
    close(socket_desc);
}


