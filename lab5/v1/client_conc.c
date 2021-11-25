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
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <sys/shm.h>
#include <semaphore.h>

int time_to_sleep = 313000; // time to sleep in client (313000 microseconds)


/* audio codec library functions */

static snd_pcm_t *mulawdev;
static snd_pcm_uframes_t mulawfrms;


#define mulawwrite(x) snd_pcm_writei(mulawdev, x, mulawfrms)



union
{
    uint32_t integer;
    unsigned char byte[2];
	
} conv;


// Close audio device.
void mulawclose(void) {
	snd_pcm_drain(mulawdev);
	snd_pcm_close(mulawdev);
}



float size_till_now=0;
char *ip_address;
int port;
char *filename;
int block_size=0;
int buffer_size=0;
int target_buf =0;
float lambda =0;
int method =0;
char *log_file;
int sock_desc;
struct sockaddr_in servaddr, cliaddr;
int count_timer=3; 
char req_packet[11];
float epsilon = 0.5;
float beta = 1.1;
float gamma_param = (float)1/313;

int num_of_blocks=0;


sem_t prod_mut ;

char recv_buffer[10000];
char recv_buffer_full[1000000];
    

sem_t conc_mut;


int full=0;
int fg=0;




float update_lambda(float lambda){


	float new_lambda = 0;
	if(method==0){

		// C
		new_lambda = lambda + epsilon * ((float)target_buf - size_till_now);



	}

	else{

		// D
		new_lambda = lambda + (epsilon * ((float)target_buf - size_till_now)/(float)block_size) - (beta*(lambda - gamma_param)*(float)(4096/(float)block_size));
	}

	return new_lambda;

}
int total=0;
int read_return=0;

// reads data if there is capacity in buffer
void produce(){

		sem_wait(&prod_mut);
		
	
		if(full+block_size<=buffer_size){
			bzero(&recv_buffer, sizeof(recv_buffer));
		
			socklen_t len = sizeof(servaddr);
			read_return = recvfrom(sock_desc, recv_buffer, block_size, 
				0, ( struct sockaddr *) &servaddr, &len);
			total+= read_return;

			if(fg==0){
					struct sigaction sa;
					struct itimerval times;
					times.it_value.tv_sec = 0;
					times.it_value.tv_usec = 0;
					times.it_interval.tv_sec = 0;
					times.it_interval.tv_usec = 0;
					int ret = setitimer (ITIMER_REAL, &times, NULL);
					printf("Established Connection with Server child of IP: %s and port: %i\n",
	           inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));
					fg=1;
				}
			
			strcat(recv_buffer_full,recv_buffer );
			full+=read_return;
			


		}

		sem_post(&prod_mut);


}


// writes to audio device if there is enough data (>=4096) in buffer
void consume(){

	sem_wait(&conc_mut);

	

	if(full>=block_size){
		

		
		char first_read [block_size];
		bzero(&first_read, sizeof(first_read));
		strcpy(first_read, recv_buffer_full);

		int k=block_size;
		for(int y=0;y<buffer_size && k<buffer_size;y++){
			recv_buffer_full[y]=recv_buffer_full[k++];
		}
		full=full-block_size;
		mulawwrite(first_read);
		usleep(time_to_sleep);

	}
	// else we ignore the data to write to audio device


	
	
	sem_post(&conc_mut);

}


void mulawopen(size_t *bufsiz) {
	snd_pcm_hw_params_t *p;
	unsigned int rate = 8000;

	snd_pcm_open(&mulawdev, "default", SND_PCM_STREAM_PLAYBACK, 0);
	snd_pcm_hw_params_alloca(&p);
	snd_pcm_hw_params_any(mulawdev, p);
	snd_pcm_hw_params_set_access(mulawdev, p, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(mulawdev, p, SND_PCM_FORMAT_MU_LAW);
	snd_pcm_hw_params_set_channels(mulawdev, p, 1);
	snd_pcm_hw_params_set_rate_near(mulawdev, p, &rate, 0);
	snd_pcm_hw_params(mulawdev, p);
	snd_pcm_hw_params_get_period_size(p, &mulawfrms, 0);
	*bufsiz = (size_t)mulawfrms;
	return;
}

// checks argument provided by client
// returns 0 if success
// return -1 if not
int check_param(){
	
	
	 
	
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
void resend_req (int sig) {
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
		sendto(sock_desc, req_packet, sizeof(req_packet), 
        0, (const struct sockaddr *) &servaddr,
            len);
	    printf("Client sent the request again to server - %d,%s\n\n",block_size,filename );

    	struct sigaction sa;
		struct itimerval times;
		sa.sa_handler = resend_req;
		sigaction (SIGALRM, &sa, NULL);

		times.it_value.tv_sec = 0;
		times.it_value.tv_usec = 2000*1000;
		times.it_interval.tv_sec = 0;
		times.it_interval.tv_usec = 0;
		int ret = setitimer (ITIMER_REAL, &times, NULL);
		printf("timer for 2 seconds initiated again\n");


    	
    }


}



int main(int argc, char *argv[])
{

	if(argc!=10){
		printf("%s\n","please provide all the 10 command line arguments as below - " );
		printf("% audiocli srv-ip srv-port audiofile blocksize buffersize targetbuf lambda method logfilecli\n" );
		printf("**********************************\n");
		return 0;
	}
	ip_address = argv[1];
	port = atoi(argv[2]);
	filename = argv[3];
	block_size = atoi(argv[4]);
	buffer_size = atoi(argv[5]);
	target_buf = atoi(argv[6]);
	lambda = atof(argv[7]);
	method = atoi(argv[8]);
	log_file = argv[9];

	num_of_blocks = buffer_size/block_size;

	printf("%ld, %ld\n",sizeof(recv_buffer), sizeof(recv_buffer_full));
	full = 0;

	if(sem_init(&prod_mut,1,1) < 0)
	{
		perror(" prod semaphore initilization");
		exit(0);
	}

	if(sem_init(&conc_mut,1,1) < 0)
	{
		perror("consumer semaphore initilization");
		exit(0);
	}

	// to check if the filename given is valid
	if(check_param(filename)==-1)
	   exit(1);
	
	  const char s[2] = ".";
   		char *token;
   
   		
		char fname[8];
		strcpy(fname, filename);
		
   		token = strtok(fname, s);
   		int dots=0;
		char* pr_token;
		
   		while( token != NULL ) {
			pr_token = token;
    		dots++;
      		token = strtok(NULL, s);
   		}
		dots--;
		printf("dots = %d, %s\n",dots, pr_token);
   		if(dots>1)
   		{
   			printf("filename is not valid\n");
   			exit(1);
   		
   		}

   		if(strcmp(pr_token,"au")!=0){
   			printf("filename does not have proper extension\n");
   			exit(1);
   	
   		}

   		

   		conv.integer = block_size;

   		int u=0;
		for(u=0;u<2;u++)
			req_packet[u]=conv.byte[u];

		printf("filename here %s \n", filename);
		
		for(int y=0;y<8;y++){
			req_packet[u++]=(unsigned char) filename[y];
		}

		
		
	
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
	    servaddr.sin_addr.s_addr = inet_addr(ip_address);
	    servaddr.sin_port = htons(port);


	    if(count_timer>0){

	    	initiate_socket();
			bind_to_socket();

			socklen_t len = sizeof(servaddr);
			int r=sendto(sock_desc, req_packet, sizeof(req_packet), 
	        0, (const struct sockaddr *) &servaddr,
	            len);
			printf("sendto %d\n",r );
			
			printf("Client sent the request to server - %d,%s\n",block_size, filename );
			
			struct sigaction sa;
			struct itimerval times;
			sa.sa_handler = resend_req;
			sigaction (SIGALRM, &sa, NULL);

			times.it_value.tv_sec = 0;
			times.it_value.tv_usec = 2000*1000;
			times.it_interval.tv_sec = 0;
			times.it_interval.tv_usec = 0;
			int ret = setitimer (ITIMER_REAL, &times, NULL);
			size_t bufsize = (size_t)buffer_size;  

			char file_dest[100];
			bzero(&file_dest, sizeof(file_dest));

			

			
			
			strcat(file_dest,filename);
			

			printf("file destination %s\n", file_dest);
		
			mulawopen(&bufsize);
			
			char rem[block_size];
			bzero(&rem, sizeof(rem));
			mulawwrite(rem);
			mulawclose();
			mulawopen(&bufsize);


			FILE* fp = fopen(log_file,"w");
			float size_array[1000];
			struct timeval current_time;
			time_t time1,time2;
			gettimeofday(&current_time, NULL);
			time1 = current_time.tv_sec*1000000+current_time.tv_usec;
			double time_array[1000];
			int it=0;
			bzero(&recv_buffer_full, sizeof(recv_buffer_full));
			
			while(1){
				produce();
				
				consume();

				if(read_return==1)
				{
					printf("reached the end\n");
					break;
				}


				if(read_return>=block_size)
				{
					size_till_now += (float)read_return;
					

					size_array[it]=size_till_now;
					if(it==0)
						time_array[it]=0;
					else{
						gettimeofday(&current_time, NULL);
						time2 = current_time.tv_sec*1000000+current_time.tv_usec;
						double diff = (double)(time2 - time1)/1000.00;
						time_array[it]=diff;
					}
					it++;
					if(size_till_now==buffer_size)
					{
						size_till_now =0;
						
						
						bzero(&recv_buffer_full, sizeof(recv_buffer_full));
						
					}
					
				}
				else{
				  printf("ignoring the data\n");
				}

				// updating lambda
				float new_lambda = update_lambda(lambda);


				lambda = new_lambda;

				conv.integer = floor(lambda);

				
				unsigned char ack[2];

				for(int y=0;y<2;y++)
					ack[y]=conv.byte[y];
				
			
				sendto(sock_desc, ack, sizeof(ack), 
	        0, (const struct sockaddr *) &servaddr,
	            sizeof(servaddr));
			}
			printf("Client gets filesize = %d\n", total);

			char log[1000000];
			bzero(&log, sizeof(log));
			for(int y=0;y<it;y++){
				char num[1000];
				bzero(&num, sizeof(num));
				sprintf(num,"%lf",size_array[y]);
				
				char tim[1000];
				bzero(&tim, sizeof(tim));
				sprintf(tim,"%lf\n",time_array[y]);

				strcat(log,num);
				strcat(log," ");
				strcat(log,tim);
				strcat(log,"\n");
				
			}
			fprintf(fp,"%s",log);
			
			fclose(fp);

			mulawclose();

	    }

		sem_destroy(&prod_mut);
		sem_destroy(&conc_mut);

	return 0;
}


