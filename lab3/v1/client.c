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
    servaddr.sin_addr.s_addr = inet_addr(ip_address);
    servaddr.sin_port = htons(atoi(port));
  
    if (connect(sock_desc, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
}

int count_timer=3;

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

	    write(sock_desc,req_buffer,sizeof(req_buffer));
	    printf("writing done \n");

    	signal(SIGALRM, term_prog);
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
	int secret_key = atoi(argv[4]);
	char* filename = argv[3];

	ssize_t read_return;

	


	
	conv.integer = secret_key;

	int u=0;
	for(u=0;u<2;u++)
		req_buffer[u]=conv.byte[u];
	
	
	for(u=0;u<strlen(filename);u++)
	{
		
		req_buffer[2+u]=(unsigned char)filename[u];
	}

	for(int y=(2+u);y<10;y++)
		req_buffer[y]=(unsigned char)'\0';

	for(u=0;u<10;u++)
		printf("%x\n",req_buffer[u] );


	if(count_timer>0){
		initiate_socket();
		connect_to_socket();

		write(sock_desc,req_buffer,sizeof(req_buffer));
		printf("Client sent the request to server - %d,%s\n",secret_key,filename );
		
		bool fg=0;
		int total_file_size=0;

		struct timeval current_time;
		gettimeofday(&current_time, NULL);
		time_t time1 = current_time.tv_sec*1000000+current_time.tv_usec;
	   	while(1) {

	   		char buffer[block_size];
	   		bzero(&buffer, sizeof(buffer));
	   		signal(SIGALRM, term_prog);
	   		alarm(2);
	        read_return = read(sock_desc, buffer, block_size);

	        if(read_return==0)
	        	break;
	        else if (read_return == -1) {
	        	break;
	        }
	        else
	        {
	        	alarm(0);
	        	fg=1;
	        	total_file_size+=read_return;

	        }
	        printf("%s\n",buffer );
	        printf("************************\n");
	    } 
	    gettimeofday(&current_time, NULL);
		time_t time2 = current_time.tv_sec*1000000+current_time.tv_usec;

		if(fg==1){
			alarm(0);
			printf("total_file_size = %d\n",total_file_size );
		    int completion_time = (time2-time1)/1000;

		    printf("Completion Time = %d ms\n", completion_time );
		    printf("Throughput = %f byte/ms \n",(float)total_file_size/completion_time );
		}

		close_socket();
	}
	
	
	
	

}