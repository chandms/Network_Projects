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
#include <errno.h>
#include <netdb.h>
#include <math.h>


#define SA struct sockaddr

// interger byte conversion
union
{
    uint32_t integer;
    unsigned char byte[4];
} conv;



char* port;
char* ip_address;
int sock_desc;
struct sockaddr_in servaddr, cliaddr;
unsigned char req_buffer[13];
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
char *ipad;
unsigned int client_ip=0;

unsigned int bbdecode(int secret_key, unsigned int client_ip){
	return (unsigned int)secret_key ^ client_ip;
}

void check_host_name(int hostname) { //This function returns host name for local computer
   if (hostname == -1) {
      perror("gethostname");
      exit(1);
   }
}
void check_host_entry(struct hostent * hostentry) { //find host info from host name
   if (hostentry == NULL){
      perror("gethostbyname");
      exit(1);
   }
}
void IP_formatter(char *IPbuffer) { //convert IP string to dotted decimal format
   if (NULL == IPbuffer) {
      perror("inet_ntoa");
      exit(1);
   }
}

unsigned int convert(){

   char host[256];
   
   struct hostent *host_entry;
   int hostname;
   hostname = gethostname(host, sizeof(host)); //find the host name
   check_host_name(hostname);
   host_entry = gethostbyname(host); //find host information
   check_host_entry(host_entry);
   ipad = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
   ipad="127.0.0.1";
	int i=0;
	unsigned int mult = 3;
	unsigned int sum=0;
	int cnt=0;
	while(ipad[i]!='\0'){
		unsigned int f = pow(256,mult);
		int j=i;
		char res[4];
		int k=0;
		while(ipad[j]!='\0' && ipad[j]!='.'){
			res[k]=ipad[j];
			j++;
			k++;
		}
		cnt++;
		res[k]='\0';
		sum+=((unsigned int)atoi(res)*f);
		mult--;
		j++;
		i=j;
		if(cnt==4)
			break;
	}
	printf("ip %s converted to unsigned int %u\n",ipad, sum );

	return sum;
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
    cliaddr.sin_addr.s_addr = INADDR_ANY;
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

    	for(int u=0;u<13;u++)
    		printf("%x\n",req_buffer[u] );
    	printf("\n");
	    int len = sizeof(servaddr);
		sendto(sock_desc, req_buffer, sizeof(req_buffer), 
        0, (const struct sockaddr *) &servaddr,
            len);
	    printf("Client sent the request encrypted secret key and filename %s to server again  \n\n",filename );

    

    	struct sigaction sa;
		struct itimerval times;
		sa.sa_handler = term_prog;
		sigaction (SIGALRM, &sa, NULL);

		times.it_value.tv_sec = 0;
		times.it_value.tv_usec = 500*1000;//timeout*1000;
		times.it_interval.tv_sec = 0;
		times.it_interval.tv_usec = 0;
		int ret = setitimer (ITIMER_REAL, &times, NULL);
		printf ("main:setitimer ret = %d\n", ret);


    	
    }


}



int main(int argc, char* argv[]){

	if(argc<6){
		printf("%s\n","please provide all the 6 command line arguments as below - " );
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

	

	client_ip = convert();

	// converting the secret key to 2 bytes
	conv.integer = bbdecode(secret_key,client_ip );

	printf("%x\n",conv.integer );

	int u=0;
	for(u=0;u<4;u++)
		req_buffer[u]=conv.byte[u];
	
	
	// converting the filename to unsigned chars
	for(u=0;u<strlen(filename);u++)
	{
		
		req_buffer[4+u]=(unsigned char)filename[u];
	}
	unsigned int  check_test = conv.byte[0] | ( (int)conv.byte[1] << 8 ) | ( (int)conv.byte[2] << 16 ) | ( (int)conv.byte[3] << 24 );
	printf("check %u\n",check_test );
	// forming the 10 bytes request
	for(int y=(4+u);y<13;y++)
		req_buffer[y]=(unsigned char)'\0';

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_address);
    servaddr.sin_port = htons(atoi(port));


	if(count_timer>0){
		// establishing the connection with server
		initiate_socket();
		bind_to_socket();

		for(int u=0;u<13;u++)
    		printf("%x\n",req_buffer[u] );
    	printf("\n");

		socklen_t len = sizeof(servaddr);
		int r=sendto(sock_desc, req_buffer, sizeof(req_buffer), 
        0, (const struct sockaddr *) &servaddr,
            len);

		/*for(int i=0;i<13;i++)
			printf("%x\n", req_buffer[i]);*/
		//printf("sendto %d\n",r );
		
		printf("Client sent the encrypted secret key and filename %s to server ",filename );
		
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
		times.it_value.tv_usec = 500*1000;//timeout*1000;
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
	   			printf("pos_copy = %ld\n",pos_copy );
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

		        	int e=0;
		        	printf("secret key %d\n",secret_key );
		        	for(int k=1;k<read_return-1;k++)
		        	{
		        		
		        		char f = (char) buffer[k];
		        		unsigned int fi = ( unsigned int)f;
		        		unsigned int ob = bbdecode(secret_key, fi);
		        		server_msg[e]=(char)ob;
		        	
		        		e++;
		        	}

		        	int padding =0;
		        	if(read_return==(block_size+2))
		        	{
		        		printf("coming here \n");
		        		char f = (char) buffer[block_size+1];
		        		printf("%x, %c\n",buffer[block_size+1], f);
		        		unsigned int fi = ( unsigned int)f;
		        		printf("%u\n",fi );
		        		unsigned int ob = bbdecode(secret_key, fi);
		        		padding = ob;
		        		printf("%u\n", padding);
		        		int y=read_return-3;
		        		while(y>=0 && server_msg[y]==f){
		        			server_msg[y]='\0';
		        			y--;
		        		}
		        	}
		        	else
		        	{
		        		char f = (char) buffer[read_return-1];
		        		unsigned int fi = ( unsigned int)f;
		        		unsigned int ob = bbdecode(secret_key, fi);
		        		server_msg[read_return-1]=(char)ob;
		        	}
		        	//server_msg[e]='\0';

		        	
		        	//printf("%s\n",server_msg );

		        	fprintf(fptr,"%s",server_msg);

		        	printf("***************************************\n");
		        	
		        	
		        	if(read_return<block_size || (read_return==block_size+2))
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