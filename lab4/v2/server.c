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
#define CLREQ 13
#define SA struct sockaddr
void wait_ack(int sig);

// integer byte converter

unsigned int bbencode(unsigned int res, int pub){
	return res ^ (unsigned int )pub;
}

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


int port = 0;
char* ip_address;
int secrect_key = 0;
int block_size = 0;
int timeout = 0;
char fname_recv[8];

int socket_desc, connfd;
socklen_t len;
struct sockaddr_in servaddr, cliaddr;
unsigned char buff[CLREQ];

int max_seq_numbr=0;
int cur_seq=0;
FILE *fp;
int cur_max_win=0;
int prev_cur_seq =0;
int end_of_the_file =0;
FILE* fp_copy;
int windowsize=0;
int prev_max_win_size=0;
unsigned long pos_copy=0, pos=0;
unsigned int pub_key =0;

void send_file_info_to_client(){

	while(end_of_the_file==0){

		
		
		printf("starting again \n");
		
		pos_copy = ftell(fp);

		prev_cur_seq=cur_seq;
		prev_max_win_size = cur_max_win;
		printf("*******************************\n");
		printf("%d, %d \n", cur_seq, cur_max_win);

		while(cur_seq<cur_max_win){

	    	unsigned char buffer[block_size+1];
	    	unsigned char last_pack[block_size+2];

    		char server_message[block_size];
			bzero(&buffer, sizeof(buffer));
			bzero(&server_message, sizeof(server_message));
			bzero(&last_pack, sizeof(last_pack));
			// reading file
			
			com.integer = cur_seq;
			buffer[0]=com.byte[0];
			last_pack[0]=com.byte[0];


			


	        ssize_t read_return = fread(server_message, (size_t)1, (size_t)(block_size-1), fp);

	        int u=1;
	        for(int k=0;k<read_return;k++){
	        	buffer[u]=(unsigned char)server_message[k];
	        	last_pack[u]=(unsigned char)server_message[k];
	        	u++;
	        }

	       
        	com.integer = 81; //dummy padding
        	last_pack[block_size+1]= com.byte[0];

	        


	        
	     
	        if (read_return == -1) {
	            perror("error while reading");
	            //exit(EXIT_FAILURE);
	        }
	        
	
	        //printf("%s\n",buffer );
	        
	        
	        
	        if (feof(fp)){
	        	
	        	end_of_the_file=1;
	        	int val = sendto(socket_desc, last_pack, sizeof(last_pack), 
                0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
	        	break;
	        }
	        else{
	        	int val = sendto(socket_desc, buffer, sizeof(buffer), 
                0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
	        }

	        cur_seq++;


    }

    

    

    char ack[100];
    bzero(ack, sizeof(ack));

   struct sigaction sa;
	struct itimerval times;
	sa.sa_handler = wait_ack;
	sigaction (SIGALRM, &sa, NULL);

	times.it_value.tv_sec = 1;
	times.it_value.tv_usec = 0;//timeout*1000;
	times.it_interval.tv_sec = 0;
	times.it_interval.tv_usec = 0;
	int ret = setitimer (ITIMER_REAL, &times, NULL);
	printf ("main:setitimer ret = %d\n", ret);

    

	socklen_t len = sizeof(cliaddr);

    if (recvfrom(socket_desc, ack, sizeof(ack), 0,
     (struct sockaddr*)&cliaddr, &len) < 0){
    printf("Couldn't receive\n");
	continue;
	}
	
	cur_seq= cur_seq%max_seq_numbr;
    if(cur_max_win==windowsize)
    	cur_max_win = 2*windowsize;
    else
    	cur_max_win = windowsize;
	
	

	

	printf("Received ACK from client \n");
	printf("%s\n", ack);

	
	if(end_of_the_file==1){
		close(socket_desc);
		exit(1);
	}

	}
	


}

void wait_ack(int sig){
	printf("didn't receive ack \n");
	end_of_the_file=0;
	
	fclose(fp);
	fp = fopen(fname_recv,"r");
	fseek(fp,pos_copy,0);
	cur_seq = prev_cur_seq;
	cur_max_win = prev_max_win_size;


	struct sigaction sa;
	struct itimerval times;
	sa.sa_handler = wait_ack;
	sigaction (SIGALRM, &sa, NULL);

	times.it_value.tv_sec = 0;
	times.it_value.tv_usec = 0;
	times.it_interval.tv_sec = 0;
	times.it_interval.tv_usec = 0;
	int ret = setitimer (ITIMER_REAL, &times, NULL);
	printf ("main:setitimer ret = %d\n", ret);
}





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
		printf("%s\n","please provide all six command line arguments as follows - ");
		printf("%rrunners server-IP server-port blocksize windowsize timeout\n");
		printf("******************************\n");
		return 0;
	}

	port = atoi(argv[2]);
	ip_address = argv[1];
	secrect_key = 0;
	block_size = atoi(argv[3]);
	windowsize = atoi(argv[4]);
	timeout = atoi(argv[5]); // in ms
	max_seq_numbr = 2*windowsize;
	cur_seq=0;

	
    
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
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
   
    
    


    while(1){
    	printf("\n\n*****************\n");
	    bzero(&cliaddr, sizeof(cliaddr));
	    bzero(buff, CLREQ);

	    socklen_t len = sizeof(cliaddr);  //len is value/resuslt

  	
        if (recvfrom(socket_desc, buff, sizeof(buff), 0,
	         (struct sockaddr*)&cliaddr, &len) < 0){
	        printf("Couldn't receive\n");
	        return -1;
	    }




	    for(int i=0;i<13;i++)
			printf("%x\n", buff[i]);
		printf("\n");
		

	   
	   	printf("Established Connection with Client of IP: %s and port: %i\n",
	           inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
   
        	// read the message from client and copy it in buffer
        

        // decoding back the secret key
        unsigned int  sec_key_recv = buff[0] | ( (int)buff[1] << 8 ) | ( (int)buff[2] << 16 ) | ( (int)buff[3] << 24 );

        printf("unsigned int %u\n",sec_key_recv);


        FILE* ft = fopen("acl.dat","r");

        if(ft==NULL){
        	printf("acl.dat file does not exists\n");
        	exit(1);
        }

        char * line = NULL;
    	size_t lt = 0;
    	ssize_t read;

    	pub_key =-1;
        while ((read = getline(&line, &lt, ft)) != -1) {
	        int r=0;
	        char ipad[100];
	        int k=0;
	        while(line[r]!='\0' && line[r]!='\n' && line[r]!=' '){
	        	ipad[k++]=line[r++];
	        }
	        ipad[k]='\0';
	        r++;
	        //printf("verify %s, %s\n",ipad, inet_ntoa(cliaddr.sin_addr));
	        if(strcmp(inet_ntoa(cliaddr.sin_addr),ipad)==0){

	        	char pkey[100];
	        	int k=0;
	        	while(line[r]!='\0' && line[r]!='\n'){
	        		pkey[k++]=line[r++];
	        	}
	        	pkey[k]='\0';
	        	printf("%s\n",pkey );
	        	pub_key = atoi(pkey);
	        }
    	}

    printf("public key %u\n", pub_key);
    fclose(ft);
    if(pub_key==-1){
    	printf("connected client is not in record \n");
    	continue;
    }

    unsigned int encode_res = bbencode(sec_key_recv ,pub_key);
    printf("encoding result %u\n",encode_res );

   
	char conv_ip[100];
    sprintf(conv_ip, "%d.%d.%d.%d",
  (encode_res >> 24) & 0xFF,
  (encode_res >> 16) & 0xFF,
  (encode_res >>  8) & 0xFF,
  (encode_res      ) & 0xFF);
    
    

    if(strcmp(conv_ip,inet_ntoa(cliaddr.sin_addr))==0){
    	printf("obtained ip address %s matches connected client's ip %s\n", conv_ip, inet_ntoa(cliaddr.sin_addr));
    	printf("authentication successful\n");
    	continue;
    	
    }
    else{
    	printf("obtained ip address %s does not match connected client's ip %s\n", conv_ip, inet_ntoa(cliaddr.sin_addr));
    	printf("authentication failure\n");
    	continue;
    }
        

        // decoding the filename
        
        int cn=0;
        for(int u=4;u<CLREQ;u++){
        	fname_recv[cn]=(char)buff[u];
        	cn++;
        }


        printf("Received file From Client - %s\n",fname_recv);

        //break;
        // ip address check
	    //int res = ip_address_check(cliaddr);
	    //if(res==-1){
	    //	printf("Server can't process unsecured request\n");
	    //	continue;
	    //}



        //sleep(7);
    	pid_t frk;
    	int status;
    	
    	fp = fopen(fname_recv,"r");
    	if(fp==NULL){
    		printf("%s File does not exist\n",fname_recv);
    		continue;
    	}
    	
    	
    	frk = fork();
    	
    	cur_max_win=windowsize;
    	prev_cur_seq =0;
    	end_of_the_file =0;
    	if(frk==0){
    		send_file_info_to_client();
    		

    	}
    	else{
    		close(socket_desc);
    		waitpid(frk, &status, 0);
    		printf("child exited \n");
    	}
	    	
	}
   
    close(socket_desc);
}


