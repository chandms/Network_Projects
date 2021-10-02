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
#include <sys/time.h>

#define BUF_SIZE 1000

int seq=0;
int sock_descriptor;
char server_message[BUF_SIZE];
struct sockaddr_in server_addr,client_addr;
socklen_t server_struct_length, client_struct_length;
int port;
time_t st_time[BUF_SIZE];
time_t en_time[BUF_SIZE];
int n,t,d,s, packet;
int iterator=0;
int counter=0;

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
	printf("didn't recieve the response from server for last packet within last 10 seconds, so, terminating\n");
    kill (0,SIGTERM);
}

void send_message_to_server(){
	conv.integer = packet;
	com.integer = d;

	unsigned char client_message[5];
	int pc=0;
	for(pc=0;pc<4;pc++)
		client_message[pc]=conv.byte[pc];

	client_message[pc]=com.byte[0];

	memset(server_message, '\0', sizeof(server_message));

	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	time_t time1 = current_time.tv_sec;
	st_time[counter]=time1*1000000+current_time.tv_usec;
	counter++;
	

	if(sendto(sock_descriptor, client_message, sizeof(client_message), 0,
	         (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
	        printf("Unable to send message\n");
	        exit(1);
	 }

	printf("Client sent %d,%d at %ld time \n", packet,d,time1*1000000+current_time.tv_usec);
	packet++;
}

void some_prog(int sig){

	send_message_to_server();
	int nit = iterator+1;
	iterator++;

	if(nit==n){
			printf("Last message is sent, now waiting for 10 seconds to receive the response\n");
	    	signal(SIGALRM, term_prog);
	    	alarm(10);
	}
	else{
	    	signal(SIGALRM, some_prog);
	    	printf("client is sleeping for %d seconds\n",t );
	    	alarm(t);
	}

	
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
    char* token[5];

    fp = fopen("pingparam.dat","r");
	if(fp==NULL){
		printf("%s\n","pingparam.dat file doesn't exist " );
		exit(EXIT_FAILURE);
	}

	if ((read = getline(&line, &len, fp)) != -1) {

        token[0]=strtok(line,delim);
        int lc=0;
        while(lc<4){
        	lc++;
        	token[lc]=strtok(NULL,delim);
        	if(token[lc]==NULL)
        		break;
        }
    }
	n=atoi(token[0]);
    t=atoi(token[1]);
    d=atoi(token[2]);
    s=atoi(token[3]);

    printf("Read: N =%d, T= %d, D= %d, S= %d\n",n,t,d,s );

    packet = s;

    port = atoi(argv[3]);
	server_struct_length = sizeof(server_addr);

	if ( (sock_descriptor = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed in client");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    client_struct_length = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(argv[2]);

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = 0;
    client_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(bind(sock_descriptor, (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Client couldn't bind to the port\n");
        return -1;
    }
    else
    	printf("client binding done\n");


    memset(st_time,0,sizeof(st_time));
	memset(en_time,0,sizeof(en_time));

	while(1){
		if(iterator<n){

			send_message_to_server();
			int nit = iterator+1;
			iterator++;

			if(nit==n){
		    	signal(SIGALRM, term_prog);
		    	printf("Last message is sent, now waiting for 10 seconds to receive the response\n");
		    	alarm(10);
			}
			else{
		    	signal(SIGALRM, some_prog);
		    	printf("client is sleeping for %d seconds\n",t );
		    	alarm(t);
			}

		}
		int index=0;
		int recv_res = recvfrom(sock_descriptor, server_message, sizeof(server_message), 0,
		         (struct sockaddr*)&server_addr, &server_struct_length) ;
		if(recv_res<0){
		        printf("Error while receiving server's message\n");
		        //return -1;
		}
	    else{

	    	alarm(0);
	    	struct timeval current_time;
		    gettimeofday(&current_time, NULL);
	    	time_t time2 = current_time.tv_sec;
	    	int val_obtained = atoi(server_message);
	    	index = val_obtained - s;
	    	en_time[index]=time2*1000000+current_time.tv_usec;

	    	//float diff = (float)(time2-time1)*1000.00;
		    float rtt_val = (en_time[index]-st_time[index])/1000.00;
		    printf("Obtained Message from server = %s at time = %ld\n",server_message,time2*1000000+current_time.tv_usec);
		    printf("################## RTT for message %s = %f\n", server_message, rtt_val);
		}

		if(index==n-1)
			break;



	}
	

	// for(int c=0;c<n;c++)
	// 	printf("%ld, %ld, %ld\n",st_time[c],en_time[c], en_time[c]-st_time[c] );

    fclose(fp);
    close(sock_descriptor);





}