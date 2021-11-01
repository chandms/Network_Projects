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
unsigned char req_buffer[11];
int secret_key;
char* filename;
int count_timer=3; // the client can send the request at max 3 times
int windowsize = 0;
int end_of_the_file=0;

int cur_seq=0;
int max_seq_num=0;
FILE* fp_copy, *fptr;
int cur_max_win =0;
int prev_cur_seq=0;
int prev_cur_max_win=0;
unsigned long pos_copy=0, pos=0;

// checks argument provided by client
// returns 0 if success
// return -1 if not
int check_param(){
	
	// checks the secret key if it is in valid range
	 if(secret_key<0 || secret_key>65535){
	 	printf("secret key is not in range\n");
	 	return -1;
	 }
	 
	
	 int count_of_dots=0; // only one dot is valid
	 for(int i=0;i<strlen(filename);i++){
	 
	 	 // checks if the filename has only alphabets
	 	if((filename[i]>='A' && filename[i]<='Z') || (filename[i]>='a' && filename[i]<='z'))
	 		continue;
	 	else if(filename[i]=='.' && count_of_dots==0){
	 		count_of_dots=1;
	 	}
	 	else
	 	{
	 		printf("filename is not proper as specification\n");
	 		return -1;
	 	}
	 
	 }
	 // if all successful
	return 0;
}

// initiate the socket
void initiate_socket(){
	 sock_desc = socket(AF_INET, SOCK_DGRAM, 0);
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
void bind_to_socket(){
	bzero(&cliaddr, sizeof(cliaddr));
  
  	cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = 0;
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   
    if(bind(sock_desc, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) < 0){
        printf("Client couldn't bind to the port\n");
        return ;
    }
    else
    	printf("client binding done\n");
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
		exit(1);
		// closing the existing connection on terminating the request.
	}
    else{
    	
    	printf("counter timer = %d\n",count_timer );

    	// decreasing the counter each time after resending request.
    	count_timer--;

    	close_socket();
    	initiate_socket();
    	bind_to_socket();

	    int len = sizeof(servaddr);
		sendto(sock_desc, req_buffer, sizeof(req_buffer), 
        0, (const struct sockaddr *) &servaddr,
            len);
	    printf("Client sent the request again to server - %d,%s\n\n",secret_key,filename );

    	

    	struct sigaction sa;
		struct itimerval times;
		sa.sa_handler = term_prog;
		sigaction (SIGALRM, &sa, NULL);

		times.it_value.tv_sec = 0;
		times.it_value.tv_usec = 500*1000;
		times.it_interval.tv_sec = 0;
		times.it_interval.tv_usec = 0;
		int ret = setitimer (ITIMER_REAL, &times, NULL);
		printf ("main:setitimer ret = %d\n", ret);


    	
    }


}



int main(int argc, char* argv[]){

	if(argc!=7){
		printf("%s\n","please provide all the 7 command line arguments as below - " );
		printf("% rrunnerc server-IP server-port filename secret-key blocksize windowsize\n" );
		printf("**********************************\n");
		return 0;
	}

	int res=0;

	ip_address = argv[1];
	port = argv[2];
	int block_size = atoi(argv[5]);
	secret_key = atoi(argv[4]);
	filename = argv[3];
	windowsize = atoi(argv[6]);

	struct timeval current_time;
	time_t time1,time2;

	ssize_t read_return;

	// converting the secret key to 2 bytes
	conv.integer = secret_key;
	
	if(check_param()==-1)
	{
		exit(1);
	}

	int u=0;
	for(u=0;u<2;u++)
		req_buffer[u]=conv.byte[u];
	
	
	// converting the filename to unsigned chars
	for(u=0;u<strlen(filename);u++)
	{
		
		req_buffer[2+u]=(unsigned char)filename[u];
	}

	// forming the 10 bytes request
	for(int y=(2+u);y<11;y++)
		req_buffer[y]=(unsigned char)'\0';

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
    	servaddr.sin_addr.s_addr = inet_addr(ip_address);
    	servaddr.sin_port = htons(atoi(port));


	if(count_timer>0){
		// establishing the connection with server
		initiate_socket();
		bind_to_socket();

		socklen_t len = sizeof(servaddr);
		int r=sendto(sock_desc, req_buffer, sizeof(req_buffer), 
        0, (const struct sockaddr *) &servaddr,
            len);
		
		printf("Client sent the request to server - %d,%s\n",secret_key,filename );
		
		bool fg=0;
		int total_file_size=0;

		char path_to_file[100];
		getcwd(path_to_file, sizeof(path_to_file));
		strcat(path_to_file,"/Client/");
		strcat(path_to_file,filename);

		
		
		struct sigaction sa;
		struct itimerval times;
		sa.sa_handler = term_prog;
		sigaction (SIGALRM, &sa, NULL);

		times.it_value.tv_sec = 0;
		times.it_value.tv_usec = 500*1000;
		times.it_interval.tv_sec = 0;
		times.it_interval.tv_usec = 0;
		int ret = setitimer (ITIMER_REAL, &times, NULL);
		printf ("main:setitimer ret = %d\n", ret);

		
	   	unsigned char buffer[block_size+2];
	   	cur_seq=0;
	   	cur_max_win=windowsize;
	   	int seq_num=0;
	   	fptr = fopen(path_to_file,"w");
	   	pos_copy = ftell(fptr);

	   	prev_cur_max_win = windowsize;
	   	prev_cur_seq = cur_seq;
	

	   	while(1) {

	   		
	   		int st=0;
	   		printf("%d, %d\n", cur_seq, cur_max_win);
	   		while(cur_seq<cur_max_win && end_of_the_file==0)
	   		{
	   			bzero(&buffer, sizeof(buffer));

		        read_return = recvfrom(sock_desc, buffer, sizeof(buffer), 
	                0, ( struct sockaddr *) &servaddr,&len);



		        if(read_return==0)
		        	break;
		        else if (read_return == -1) {
		        	break;
		        }
		        else
		        {
		        	
		        	if(fg==0){
		        		alarm(0);
						gettimeofday(&current_time, NULL);
						time1 = current_time.tv_sec*1000000+current_time.tv_usec;

		        	}
		        	
		        	fg=1; // flag to denote if the client received any response from server

		        	total_file_size+=read_return; // summing up file_size
		        	char server_msg[block_size];
		        	bzero(server_msg,sizeof(server_msg));
		        	int seq = buffer[0];

		        	printf("got seq number %d\n", seq);
		        	if(st==0){
		        		st=1;
		        		if(seq!=cur_seq){
		        			rewind(fptr);
		        			fseek(fptr, pos_copy,0);
		        			cur_seq = prev_cur_seq;
		        			cur_max_win = prev_cur_max_win;
		        		}
		        		else{
		        			pos_copy = ftell(fptr);
		        			prev_cur_seq = cur_seq;
		        			prev_cur_max_win = cur_max_win;
		        		}
		        	}

		        	for(int k=1;k<read_return-1;k++)
		        	{
		        		server_msg[k-1]=(char)buffer[k];
		        	}

		        	int padding =0;
		        	if(read_return==(block_size+2))
		        		padding = buffer[block_size+1];
		        	else
		        		server_msg[read_return-1]=(char)buffer[read_return-1];

		        	
		        	//printf("%s\n",server_msg );

		        	fprintf(fptr,"%s",server_msg);

		        	printf("***************************************\n");
		        	
		        	
		        	if(read_return<block_size || (read_return==block_size+2 && padding==81))
		        	{
		        		end_of_the_file=1;
		        	}


		        }

		        cur_seq++;
	        
	   		}
	   		printf("got till %d, %d\n",cur_seq-1 , end_of_the_file);

	   		char seq[3];
	   		bzero(seq, sizeof(seq));
	   		sprintf( seq, "%d", cur_seq-1 );
			
			
			// if want to add randomness
	   		/*if(rand()%2==0)
	   		{
	   			printf("not sleeping\n");
	   			sleep(0);
	   		}
	   		else
	   		{
	   			printf("sleeping\n");
	   			sleep(2);
	   		}*/
	   		

            
	   		

	   		cur_seq = cur_seq%(2*windowsize);
	   		if(cur_max_win==windowsize)
	   			cur_max_win = 2*windowsize;
	   		else
	   			cur_max_win = windowsize;


	   		if(end_of_the_file==1)
	   		{
		   			for(int y=0;y<8;y++){
		   				r=sendto(sock_desc, seq, sizeof(seq), 
	        0, (const struct sockaddr *) &servaddr,
	            len);
		   			}
		   			break;
	   		}
	   		else{
		   			r=sendto(sock_desc, seq, sizeof(seq), 
	        0, (const struct sockaddr *) &servaddr,
	            len);	
	   		}




	   		
	    }

	    
	    // noting the time of last reply
	    gettimeofday(&current_time, NULL);
		time2 = current_time.tv_sec*1000000+current_time.tv_usec;

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
		fclose(fptr);

		close_socket();
	}
	
	
	
	

}
