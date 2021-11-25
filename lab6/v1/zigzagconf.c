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
#include "zigzagconf.h" // adding the header file 


/*
        zigzagconf

*/
int ctrl_sock_desc;
struct sockaddr_in ctrl_servaddr, ctrl_cliaddr;


// create the socket
void create_the_socket(){
	 ctrl_sock_desc = socket(AF_INET, SOCK_DGRAM, 0);
	    if (ctrl_sock_desc == -1) {
	        printf("socket creation failed...\n");
	        exit(0);
	    }
	    else
	        printf("Socket successfully created..\n");
}

// close the socket
void close_the_socket(){
	close(ctrl_sock_desc);
}

// bind  to socket
void bind_to_the_socket(){
	bzero(&ctrl_cliaddr, sizeof(ctrl_cliaddr));
  
  	ctrl_cliaddr.sin_family = AF_INET;
    ctrl_cliaddr.sin_port = 0;
    ctrl_cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   
    if(bind(ctrl_sock_desc, (struct sockaddr*)&ctrl_cliaddr, sizeof(ctrl_cliaddr)) < 0){
        printf("zigzagconf couldn't bind to the port\n");
        return ;
    }
    else
    	printf("zigzagconf binding done\n");
}

// to convert to two bytes
union
{
    uint32_t integer;
    unsigned char byte[2];
} conv_two_bytes;

// to convert to four bytes
union
{
    uint32_t integer;
    unsigned char byte[4];
} conv_four_bytes;

// route table
struct ctrl_route_table{
    int data_real_port;
    int data_forward_port_listen;
    int data_forward_port;
    int data_backward_port_listen;
    int data_backward_port;

    char data_real_ip[100];
    char data_forward_ip[100];
    char data_backward_ip[100];

};

char* tokens[100]; // tokenize the line

void parse_line(char line[100], char delim[2]){

    for(int i=0;i<100;i++)
       tokens[i]=NULL;

	

	int i=0;
	tokens[0]=strtok(line,delim);
	while(tokens[i]!=NULL){
		i++;
		tokens[i]=strtok(NULL,delim);
	}
}

void zig_zag_conf(){

    create_the_socket();
    bind_to_the_socket();
    

    FILE *fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("zzoverlay.dat","r");

    if(fp==NULL){
        printf("zzoverlay file does not exist\n");
        return ;

    }
    struct ctrl_route_table route_info[100];

    int fg=0;
    int routers=0;
    int rt_it=0;
     while (1) {
         
        struct ctrl_route_table rt;
        
        if(fg==0){
            read = getline(&line, &len, fp);
            routers = atoi(line);
            fg=1; 
            
        }
        else{
            
            int count_iterator=0;
            while(count_iterator<3 && (read = getline(&line, &len, fp)) != -1 && line!=NULL){
                char delim[2]=" ";
                parse_line(line,delim);
                if(count_iterator==0){
                    strcpy(rt.data_real_ip,tokens[0]);
                    rt.data_real_port = atoi(tokens[1]);

                }
                else if(count_iterator==1){

                    strcpy(rt.data_forward_ip,tokens[2]);
                    rt.data_forward_port_listen = atoi(tokens[0]);
                    rt.data_forward_port = atoi(tokens[1]);

                }
                else if(count_iterator==2){
                    strcpy(rt.data_backward_ip,tokens[2]);
                    rt.data_backward_port_listen = atoi(tokens[0]);
                    rt.data_backward_port = atoi(tokens[1]);
                }
                count_iterator++;
            }
            route_info[rt_it++]=rt;


            // printf("%s, %s, %s\n", rt.real_ip, rt.forward_ip, rt.backward_ip);
            // printf("%d, %d, %d , %d, %d\n", rt.real_port, rt.forward_port_listen, rt.forward_port, 
            // rt.backward_port_listen, rt.backward_port);

            // printf("**********************************************\n");

        }
        if(read==-1)
          break;
    }


    printf("num = %d\n", routers);


    for(int i=0;i<routers;i++){

            // printf("%s, %s, %s\n", route_info[i].real_ip, route_info[i].forward_ip, route_info[i].backward_ip);
            // printf("%d, %d, %d , %d, %d\n", route_info[i].real_port, route_info[i].forward_port_listen, route_info[i].forward_port, 
            // route_info[i].backward_port_listen, route_info[i].backward_port);

            unsigned char udp_packet_management[17];
            char delim[2]=".";

            conv_two_bytes.integer = route_info[i].data_forward_port_listen;
            udp_packet_management[0]=conv_two_bytes.byte[0];
            udp_packet_management[1]=conv_two_bytes.byte[1];

            conv_two_bytes.integer = route_info[i].data_forward_port;
            udp_packet_management[2]=conv_two_bytes.byte[0];
            udp_packet_management[3]=conv_two_bytes.byte[1];


            char for_ip[100];
            bzero(&for_ip, sizeof(for_ip));
            strcpy(for_ip,route_info[i].data_forward_ip);

            
            parse_line(route_info[i].data_forward_ip,delim);
            unsigned int ip_sum=0;

            

            int pw=3;
            for(int j=0;j<4;j++){
                unsigned int x = pow(256,pw);
                ip_sum+=(x*atoi(tokens[j]));
                pw--;
            }

            printf(" forward ip (in unsigned int format) %u, %s\n",ip_sum, for_ip);

            conv_four_bytes.integer = ip_sum;
            udp_packet_management[4]= conv_four_bytes.byte[0];
            udp_packet_management[5]= conv_four_bytes.byte[1];
            udp_packet_management[6]= conv_four_bytes.byte[2];
            udp_packet_management[7]= conv_four_bytes.byte[3];

            conv_two_bytes.integer = route_info[i].data_backward_port_listen;
            udp_packet_management[8]=conv_two_bytes.byte[0];
            udp_packet_management[9]=conv_two_bytes.byte[1];

            conv_two_bytes.integer = route_info[i].data_backward_port;
            udp_packet_management[10]=conv_two_bytes.byte[0];
            udp_packet_management[11]=conv_two_bytes.byte[1];

            char back_ip[100];
            bzero(&back_ip, sizeof(back_ip));
            strcpy(back_ip,route_info[i].data_backward_ip);

            parse_line(route_info[i].data_backward_ip,delim);
            ip_sum=0;

            pw=3;
            for(int j=0;j<4;j++){
                unsigned int x = pow(256,pw);
                ip_sum+=(x*atoi(tokens[j]));
                pw--;
            }

            printf(" back ip (in unsigned int format) %u, %s\n",ip_sum, back_ip);

            printf("************************************\n");

            conv_four_bytes.integer = ip_sum;
            udp_packet_management[12]= conv_four_bytes.byte[0];
            udp_packet_management[13]= conv_four_bytes.byte[1];
            udp_packet_management[14]= conv_four_bytes.byte[2];
            udp_packet_management[15]= conv_four_bytes.byte[3];

            udp_packet_management[16]=(unsigned char)'\0';

            bzero(&ctrl_servaddr, sizeof(ctrl_servaddr));
            ctrl_servaddr.sin_family = AF_INET;
            ctrl_servaddr.sin_addr.s_addr = inet_addr(route_info[i].data_real_ip);
            ctrl_servaddr.sin_port = htons(route_info[i].data_real_port);


            



    

            int len = sizeof(ctrl_servaddr);
		sendto(ctrl_sock_desc, udp_packet_management, sizeof(udp_packet_management), 
        0, (const struct sockaddr *) &ctrl_servaddr,
            len);


            printf("sent the packet to %s, %d\n", route_info[i].data_real_ip, route_info[i].data_real_port);






    }
    close_the_socket();

}