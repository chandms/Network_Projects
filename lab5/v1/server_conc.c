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
#include <sys/time.h> 
#include <alsa/asoundlib.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/shm.h>
  


#define SA struct sockaddr

#define CLREQ 11  // for request buffer size

union
{
    uint32_t integer;
    unsigned char byte[2];
} conv;

/* audio codec library functions */

static snd_pcm_t *mulawdev;
static snd_pcm_uframes_t mulawfrms;


int socket_desc, connfd;
socklen_t len;
struct sockaddr_in servaddr, cliaddr;
char* ip_address;
int port;
int init_lambda;
int file_extension=1;
char* filename;
unsigned char client_request[CLREQ];
int block_size;

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

// Close audio device.
void mulawclose(void) {
	snd_pcm_drain(mulawdev);
	snd_pcm_close(mulawdev);
}


// ip address check to check if the client is from authentic domain
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

	if(!(strcmp(tk[0],"128")==0 && strcmp(tk[1],"10")==0 && (strcmp(tk[2],"25")==0 || strcmp(tk[2],"112")==0)))
	{
		printf("ip address invalid \n");
	 	return -1;
	}

	printf("IP address check successful \n");

	return 1;

}


int main(int argc, char* argv[]){

	if(argc!=5){
		printf("%s\n","please provide all five command line arguments as follows - ");
		printf("%audiosrv srv-IP srv-port lambda logfilesrv\n");
		printf("******************************\n");
		return 0;
	}

	int shm_id=1;

	// creating shared variable to distinguish between different server log files for different clients
	shm_id = shmget(IPC_PRIVATE, sizeof(sem_t) + 4*sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
         printf("shmget error\n");
    }
    int *shmpointer = shmat(shm_id,0,0);

	
    sem_t mutex ; // semaphore to generate file extension for log files
    
    if(sem_init(&mutex,1,1) < 0)
    {
        perror("semaphore initilization");
        exit(0);
    }        

	//sem_init(&mutex, 0, 1);

	ip_address = argv[1];
	port = atoi(argv[2]);
	init_lambda = atof(argv[3]);
	filename = argv[4];


	// socket created
	socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
   
   	

    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_address);
    servaddr.sin_port = htons(port);


    if ((bind(socket_desc, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    while(1){

    	printf("waiting for clients \n\n*****************\n");
	    bzero(&cliaddr, sizeof(cliaddr));
	    bzero(client_request, CLREQ);

	    socklen_t len = sizeof(cliaddr);  //len is value/resuslt

		// receiving request from client ( blocksize & filename)
        if (recvfrom(socket_desc, client_request, sizeof(client_request), 0,
	         (struct sockaddr*)&cliaddr, &len) < 0){
	        printf("Couldn't receive\n");
	        return -1;
	    }

		if(ip_address_check(cliaddr)==-1)
		   continue;

		// computing block size
	    block_size = (client_request[1]<<8)+client_request[0];

	    
		
	    char fname_recv[8], fname_cpy[8];
	    int cn=0;
        for(int u=2;u<CLREQ;u++){
        	fname_recv[cn]=(char)client_request[u];
        	cn++;
        }

		strcpy(fname_cpy, fname_recv);

		
        const char s[2] = ".";
   		char *token;
   
   		/* get the first token */
   		token = strtok(fname_cpy, s);
		char* pr_token;
   		int dots=0;

   		while( token != NULL ) {
			pr_token = token;
    		dots++;
      		token = strtok(NULL, s);
   		}
		dots--;
   		if(dots>1)
   		{
   			printf("filename is not valid\n");
   			continue;
   		}
		printf("pr token %s",pr_token);
   		if(strcmp(pr_token,"au")!=0){
   			printf("filename does not have proper extension\n");
   			continue;
   		}

   		printf("received filename = %s, block size = %d\n",fname_recv,block_size );

   		FILE *fp = fopen(fname_recv,"r");

   		if(fp==NULL){
   			printf("%s file does not exist\n",fname_recv);
   			continue;
   		}

   		

   		printf("block size obtained = %d\n",block_size );

		
   		pid_t fk;
   		int status;
		/// if all checks above are successful, then fork one child to specifically connect to the client

		

   		fk = fork();

   		if(fk==0){
			int lambda_array[1000]; // to store all the lambda changes in log file
			int it=0;
			
			// storing the file extension value for this child
			sem_wait(&mutex);
			int current_num = (*shmpointer+1);
			*shmpointer += 1;
			
			//file_extension++;
			printf("curr num = %d, file_extension =  %d\n", current_num, *shmpointer);

			sem_post(&mutex);
			
		

   			int socket_desc2;
			socklen_t len;
			struct sockaddr_in servaddr2, cliaddr2;

			cliaddr2 = cliaddr;

			// creating  child socket
			socket_desc2 = socket(AF_INET, SOCK_DGRAM, 0);
		    if (socket_desc2 == -1) {
		        printf("socket creation failed by child...\n");
		        exit(0);
		    }
		    else
		        printf("Child created Socket successfully ..\n");


		    servaddr2.sin_family = AF_INET;
    		servaddr2.sin_addr.s_addr = inet_addr(ip_address);
    		servaddr2.sin_port = 0;

			
			// binding child socket
			 if ((bind(socket_desc2, (SA*)&servaddr2, sizeof(servaddr2))) != 0) {
        			printf("socket bind failed in child...\n");
        			exit(0);
    			}
    		else
        		printf("Socket successfully binded in child..\n");




   			int lambda = init_lambda;
   			size_t bufsize = block_size;
   			char *buf;	
   			



   			int fd1=0;

   			if((fd1 = open(fname_recv, O_RDONLY)) == -1) {
   				exit(1);
			}

			mulawopen(&bufsize);
   			buf = (char *)malloc(bufsize);
			struct timeval current_time;
			time_t time1,time2;
			gettimeofday(&current_time, NULL);
			time1 = current_time.tv_sec*1000000+current_time.tv_usec;
			double time_array[1000]; // to store all the time intervals in log file
			int size =0;

			// reading the audio file line by line
			while (read(fd1, buf, bufsize) > 0) {
					size+=strlen(buf);
					if(it==0)
						time_array[it]=0;
					else{
						gettimeofday(&current_time, NULL);
						time2 = current_time.tv_sec*1000000+current_time.tv_usec;
						double diff = (double)(time2 - time1)/1000.00;
						time_array[it]=diff;
					}
					
					lambda_array[it]=lambda;
					it++;
				
					int val = sendto(socket_desc2, buf, block_size, 
                0, (struct sockaddr *) &cliaddr2, sizeof(cliaddr2));
				

					unsigned char ack[2];
					socklen_t len2 = sizeof(cliaddr2);
					if (recvfrom(socket_desc2, ack, sizeof(ack), 0,
				         (struct sockaddr*)&cliaddr2, &len) < 0){
				        printf("Couldn't receive\n");
	    			}


					int ack_rec = (ack[1]<<8)+ack[0];	
	    			float pspacing = (float)1/lambda;
					int lm =lambda;
	    			lambda = ack_rec;


				struct timespec tim1, tim2;
				tim2.tv_sec=0;
				tim2.tv_nsec= pspacing*1000000;
	    			
				nanosleep(&tim2, &tim1);
						
			}
			//printf("current buf = %d", size);
			char fin[1];
			fin[0]='\0'; // dummy end (no payload)
			for(int it=0;it<8;it++ ){
				sendto(socket_desc2, fin, sizeof(fin), 
                0, (struct sockaddr *) &cliaddr2, sizeof(cliaddr2));
			}
			printf("8 packets are sent\n");
			close(socket_desc2);
			mulawclose();
			free(buf);
			

			// adding lambda and time intervals to log file
			char client_file_name[100];
			strcpy(client_file_name, filename);
			char num[10];
			sprintf(num, "%d", current_num);
			strcat(client_file_name, num);
			FILE* fp = fopen(client_file_name,"w");
			char log[1000000];
			bzero(&log, sizeof(log));
			for(int y=0;y<it;y++){
				char lam[100];
				bzero(&lam, sizeof(lam));
				sprintf(lam, "%d", lambda_array[y]);

				char tim[100];
				bzero(&tim, sizeof(tim));
				sprintf(tim, "%lf", time_array[y]);

				strcat(log,lam);
				strcat (log," ");
				strcat(log, tim);
				strcat(log,"\n");
			}

			//printf("writing to this %s, %s", client_file_name, log);
			
			fprintf(fp,"%s",log);
			fclose(fp);
			exit(1);
			

   		}
   		else{
			waitpid(fk, &status, 0);
    		printf("child exited \n");
   		}

		sem_destroy(&mutex);
    }
   





}
