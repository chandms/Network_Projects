#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <math.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>


/*  
        zigzagrouter program
        run this in all hops specified in zzoverlay.dat
*/


int port; // stores the port given in command line argument
char data_plane_forw_ip[100]; // storing the forwarding ip address
char data_plane_back_ip[100]; // storing the backwarding ip address

int ctrl_plane_sock_desc, data_plane_forward_sock, data_plane_backward_sock;
struct sockaddr_in servaddr, cliaddr;
struct sockaddr_in data_plane_serv_fwd_addr, data_plane_cli_fwd_addr;
struct sockaddr_in data_plane_serv_bck_addr, data_plane_cli_bck_addr;


char* ipad; // to capture the own ip address
int cli_port;

// listen and forwarding port
int data_plane_backward_port_listen, data_plane_backward_port;

// listen and backwarding port
int data_plane_forward_port_listen, data_plane_forward_port;

// close zigzagrouter  sockets

// param : pass the socket to be closed
void close_zig_socket(int sock){
	close(sock);
}


// create the socket
// param : pass the socket to be created
int create_zig_socket(int sock){
	 sock = socket(AF_INET, SOCK_DGRAM, 0);
	    if (sock == -1) {
	        printf("socket creation failed...\n");
	        return -1;
	    }
	    else
	        printf("Socket successfully created..\n");

        return sock;
}



// bind the socket
// param : pass the socket to bind, num of sock (control=0, dataplane1=1, dataplane2=2), port
int bind_to_zig_socket(int sock, int sock_num){
	bzero(&servaddr, sizeof(servaddr));
    bzero(&data_plane_serv_fwd_addr, sizeof(data_plane_serv_fwd_addr));
    bzero(&data_plane_serv_bck_addr, sizeof(data_plane_serv_bck_addr));

    if(sock_num==0){

        // bind done for control plane
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = inet_addr(ipad);

        if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
            printf("zigzagrouter couldn't bind to the port ... (for control plane)\n");
            return -1;
        }
        else
            printf("zigzagrouter binding done (for control plane)\n");
        return sock;
    }
    else if(sock_num==1){

        // bind done for forward data plane
        data_plane_serv_fwd_addr.sin_family = AF_INET;
        data_plane_serv_fwd_addr.sin_port = htons(data_plane_forward_port_listen);
        data_plane_serv_fwd_addr.sin_addr.s_addr = inet_addr(ipad);

        if(bind(sock, (struct sockaddr*)&data_plane_serv_fwd_addr, sizeof(data_plane_serv_fwd_addr)) < 0){
            printf("zigzagrouter couldn't bind to the port ... (for forward data plane)\n");
            return -1;
        }
        else
            printf("zigzagrouter binding done (for forward data plane)\n");
        return sock;
    }
    else if(sock_num==2){
        
        // bind done for backward data plane
        data_plane_serv_bck_addr.sin_family = AF_INET;
        data_plane_serv_bck_addr.sin_port = htons(data_plane_backward_port_listen);
        data_plane_serv_bck_addr.sin_addr.s_addr = inet_addr(ipad);

        if(bind(sock, (struct sockaddr*)&data_plane_serv_bck_addr, sizeof(data_plane_serv_bck_addr)) < 0){
            printf("zigzagrouter couldn't bind to the port ...(for backward data plane)\n");
            return -1;
        }
        else
            printf("zigzagrouter binding done (for backward data plane)\n");
        return sock;
    }

    else
      return -1;
  
  	
   
    
}


int main(int argc, char* argv[]){

    if(argc!=2){
        printf("please provide port number associated to the control plane of the router \n");
        return 0;
    }
    port = atoi(argv[1]);

    ctrl_plane_sock_desc= create_zig_socket(ctrl_plane_sock_desc);
    
    
    char host_ipad[256];
   
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host_ipad, sizeof(host_ipad)); //find the host name
    host_entry = gethostbyname(host_ipad); //find host information
    
    ipad = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    
    ctrl_plane_sock_desc=bind_to_zig_socket(ctrl_plane_sock_desc, 0);
   
    printf("zigzagrouter: my ip address %s\n", ipad);
    
    unsigned char udp_man_packet[17];
    bzero(&cliaddr, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr); 
    bzero(&udp_man_packet, sizeof(udp_man_packet));
    
    recvfrom(ctrl_plane_sock_desc, udp_man_packet, sizeof(udp_man_packet), 
	                0, ( struct sockaddr *) &cliaddr,&len);


    data_plane_forward_port_listen = (udp_man_packet[1]<<8)+udp_man_packet[0];               
    data_plane_forward_port = (udp_man_packet[3]<<8)+udp_man_packet[2]; 

    unsigned int forward_ip = udp_man_packet[4] | ( (int)udp_man_packet[5] << 8 ) | ( (int)udp_man_packet[6] << 16 ) | ( (int)udp_man_packet[7] << 24 );
    printf("forward ip %u\n", forward_ip);

   
    sprintf(data_plane_forw_ip, "%d.%d.%d.%d",
  (forward_ip >> 24) & 0xFF,
  (forward_ip >> 16) & 0xFF,
  (forward_ip >>  8) & 0xFF,
  (forward_ip      ) & 0xFF);

  data_plane_backward_port_listen = (udp_man_packet[9]<<8)+udp_man_packet[8];               
  data_plane_backward_port = (udp_man_packet[11]<<8)+udp_man_packet[10]; 

    unsigned int backward_ip = udp_man_packet[12] | ( (int)udp_man_packet[13] << 8 ) | ( (int)udp_man_packet[14] << 16 ) | ( (int)udp_man_packet[15] << 24 );
     printf("backward ip %u\n", backward_ip);

   
    sprintf(data_plane_back_ip, "%d.%d.%d.%d",
  (backward_ip >> 24) & 0xFF,
  (backward_ip >> 16) & 0xFF,
  (backward_ip >>  8) & 0xFF,
  (backward_ip      ) & 0xFF);

    int first_hop =0;
    

    


    if(strcmp("0.0.0.0",data_plane_back_ip)==0){
        first_hop=1;
        
    }



    time_t capt_time; 
    capt_time=time(NULL); 
    printf("Control plane data received at %s",asctime( localtime(&capt_time) ) );

    printf("************************ Date Plane Routing Information for %s ************************\n",ipad);

    printf("                      || listen port            || forward/backward port           || ip                       \n");
    printf("Forwarding Info       || %d                  || %d           || %s                       \n",data_plane_forward_port_listen, data_plane_forward_port, data_plane_forw_ip);
    printf("Backwarding Info      || %d                  || %d           || %s                       \n",data_plane_backward_port_listen, data_plane_backward_port, data_plane_back_ip);


    printf("************************ End of Routing Table *******************************************\n");

    data_plane_forward_sock = create_zig_socket(data_plane_forward_sock);
    data_plane_backward_sock = create_zig_socket(data_plane_backward_sock);
    data_plane_forward_sock = bind_to_zig_socket(data_plane_forward_sock,1);
    data_plane_backward_sock = bind_to_zig_socket(data_plane_backward_sock,2);

    

    fd_set read_sock;
    struct timeval timeout;
    int ret=0;
    int smax = fmax(data_plane_forward_sock, data_plane_backward_sock);

    struct sockaddr_in cli_spec;


    int check=0;
    

    while(1)
    {
        FD_ZERO(&read_sock);
        FD_SET(data_plane_forward_sock, &read_sock);
        FD_SET(data_plane_backward_sock, &read_sock);
        

        // setting timeout for 2 seconds.
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        int retval = select(smax+1, &read_sock, NULL, NULL, &timeout);
        if (retval > 0)
        {
            if (FD_ISSET(data_plane_forward_sock, &read_sock))
            {
                unsigned char client_message[5];
                struct sockaddr_in cli;
                bzero(&cli, sizeof(cli));
                socklen_t len = sizeof(cli);
                ret = recvfrom(data_plane_forward_sock, client_message, sizeof(client_message), 
	                0, ( struct sockaddr *) &cli,&len);
                capt_time=time(NULL); /* get current cal time */
    
                
                if(ret > 0 )
                {

                  if(first_hop==1 && check==0){
                        check=1;
                        cli_port= cli.sin_port;
                        cli_spec = cli;
                        printf("I am  the first hop and client's port=%i\n",ntohs(cli_port));
                  }
                    printf("Data plane forwarded data received at %s",asctime( localtime(&capt_time) ) );
                    printf("Received message from IP (forward path): %s and port: %i\n",
	                inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));


                    uint32_t myInt1 = client_message[0] + (client_message[1] << 8) + (client_message[2] << 16) + (client_message[3] << 24);
                    uint32_t command = client_message[4];
	                printf("Message from client to server and Command = %d, %d\n\n\n",myInt1, command );

                   
                    
                    struct sockaddr_in cli2;
                      bzero(&cli2, sizeof(cli2));
                    cli2.sin_family = AF_INET;
                    cli2.sin_addr.s_addr = inet_addr(data_plane_forw_ip);
                    cli2.sin_port = htons(data_plane_forward_port);
                    int len2 = sizeof(cli2);
                    sendto(data_plane_backward_sock, client_message, sizeof(client_message), 
                    0, (const struct sockaddr *) &cli2,
                        len2);
                    printf("Sent message to IP (forward path): %s and port: %i\n",
	                inet_ntoa(cli2.sin_addr), ntohs(cli2.sin_port));
                }
                
            }

            if (FD_ISSET(data_plane_backward_sock, &read_sock))
            {
                char client_message[4];
                struct sockaddr_in cli;
                bzero(&cli, sizeof(cli));
                socklen_t len = sizeof(cli);
                ret = recvfrom(data_plane_backward_sock, client_message, sizeof(client_message), 
	                0, ( struct sockaddr *) &cli,&len);
                capt_time=time(NULL); /* get current cal time */
                if(ret > 0 )
                {
                    printf("Data plane: backward data received at %s",asctime( localtime(&capt_time) ) );
                    printf("Received message from IP (backward path): %s and port: %i\n",
	                inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

                    printf("Message from Server to Client = %d\n\n", atoi(client_message));

                    struct sockaddr_in cli2;
                      bzero(&cli2, sizeof(cli2));
                    cli2.sin_family = AF_INET;
                    cli2.sin_addr.s_addr = inet_addr(data_plane_back_ip);
                    cli2.sin_port = htons(data_plane_backward_port);
                    int len2 = sizeof(cli2);

                    

                    if(first_hop==1){

                        // if this router is the first hop, then it sends back the message to the client on
                        // the port where it first received message from client

                        sendto(data_plane_forward_sock, client_message, sizeof(client_message), 
                        0, (const struct sockaddr *) &cli_spec,
                            sizeof(cli_spec));  
                        printf("Sent message to IP (backward path): %s and port: %i\n",
	                    inet_ntoa(cli_spec.sin_addr), ntohs(cli_spec.sin_port));
                    }
                    else{

                        // otherwise, the router is uses the backward socket
                        sendto(data_plane_backward_sock, client_message, sizeof(client_message), 
                        0, (const struct sockaddr *) &cli2,
                            len2);
                        printf("Sent message to IP (backward path): %s and port: %i\n",
	                    inet_ntoa(cli2.sin_addr), ntohs(cli2.sin_port));
                    }
                }
                
            }
        }
        else if (retval < 0)
        {
            printf("error obtained in select \n");
        }
    }


    // closing the sockets
    close_zig_socket(ctrl_plane_sock_desc);
    close_zig_socket(data_plane_forward_sock);
    close_zig_socket(data_plane_backward_sock);

}
