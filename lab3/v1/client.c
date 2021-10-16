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
#include <sys/time.h>


#define SA struct sockaddr

// interger byte conversion
union
{
    uint32_t integer;
    unsigned char byte[2];
} conv;



char* port;
char* ip_address;
int sock_desc;
struct sockaddr_in servaddr, cliaddr;
unsigned char req_buffer[10];
int secret_key;
char* filename;
int count_timer=3; // the client can send the request at max 3 times

// initiate the socket
void initiate_socket(){
	 sock_desc = socket(AF_INET, SOCK_STREAM, 0);
	    if (sock_desc == -1) {
	        printf("socket creation failed...\n");
	        exit(0);
	    }
	    else
	        printf("Socket successfully created..\n");
}

// close the socket
void close_socket(){
	close(sock_desc);
}

// connect to server
void connect_to_socket(){
	bzero(&servaddr, sizeof(servaddr));
	   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_address);
    servaddr.sin_port = htons(atoi(port));
  
    if (connect(sock_desc, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
}


// signal handler function 
void term_prog (int sig) {
	printf("\n\n\n********************************\n");
	printf("didn't recieve the response from server\n");
	if(count_timer==0)
	{
		// if the client loses all 3 attempts of resending requests.
		printf("Repeated the request thrice, terminating Client Request Process \n");
		close_socket();
		// closing the existing connection on terminating the request.
	}
    else{
    	
    	printf("counter timer = %d\n",count_timer );

    	// decreasing the counter each time after resending request.
    	count_timer--;

    	close_socket();
    	initiate_socket();
    	connect_to_socket();

	    write(sock_desc,req_buffer,sizeof(req_buffer));
	    printf("Client sent the request again to server - %d,%s\n\n",secret_key,filename );

    	signal(SIGALRM, term_prog);
    	// reset the alarm
    	alarm(2);


    	
    }


}



int main(int argc, char* argv[]){

	if(argc<6){
		printf("%s\n","please provide all the 6 command line arguments as below - " );
		printf("% myftpc server-IP server-port filename secret-key blocksize\n" );
		printf("**********************************\n");
		return 0;
	}

	int res=0;

	ip_address = argv[1];
	port = argv[2];
	int block_size = atoi(argv[5]);
	secret_key = atoi(argv[4]);
	filename = argv[3];

	ssize_t read_return;

	// converting the secret key to 2 bytes
	conv.integer = secret_key;

	int u=0;
	for(u=0;u<2;u++)
		req_buffer[u]=conv.byte[u];
	
	
	// converting the filename to unsigned chars
	for(u=0;u<strlen(filename);u++)
	{
		
		req_buffer[2+u]=(unsigned char)filename[u];
	}

	// forming the 10 bytes request
	for(int y=(2+u);y<10;y++)
		req_buffer[y]=(unsigned char)'\0';

	


	if(count_timer>0){

		// establishing the connection with server
		initiate_socket();
		connect_to_socket();

		write(sock_desc,req_buffer,sizeof(req_buffer));
		printf("Client sent the request to server - %d,%s\n",secret_key,filename );
		
		bool fg=0;
		int total_file_size=0;

		struct timeval current_time;
		gettimeofday(&current_time, NULL);
		time_t time1 = current_time.tv_sec*1000000+current_time.tv_usec;

		FILE * fptr = fopen("file_received_by_client.txt","w");
		signal(SIGALRM, term_prog);

	   	// set alarm
	   	alarm(2);
	   	while(1) {

	   		char buffer[block_size];
	   		bzero(&buffer, sizeof(buffer));
	        read_return = read(sock_desc, buffer, block_size);

	        if(read_return==0)
	        	break;
	        else if (read_return == -1) {
	        	break;
	        }
	        else
	        {
	        	alarm(0);
	        	fg=1; // flag to denote if the client received any response from server

	        	total_file_size+=read_return; // summing up file_size
	        	fprintf(fptr,"%s",buffer);

	        }
	        
	    } 
	    fclose(fptr);
	    gettimeofday(&current_time, NULL);
		time_t time2 = current_time.tv_sec*1000000+current_time.tv_usec;

		if(fg==1){
			alarm(0);
			printf("client received the response \n");
			printf("starting time %ld\n", time1 );
			printf("ending time %ld\n", time2 );
			printf("total_file_size = %d\n",total_file_size );
		    float completion_time = (float)(time2-time1)/1000.00;

		    printf("Completion Time = %f ms\n", completion_time );
		    printf("Throughput = %f byte/ms \n",(float)total_file_size/completion_time );
		}

		close_socket();
	}
	
	
	
	

}