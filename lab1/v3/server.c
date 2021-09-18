// C program to implement one side of FIFO
// This side writes first, then reads
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "parser.h"

#define PIPE_BUF 4096
#define SERVER_NAME "commandserver"

extern char* token[PIPE_BUF];
extern char* req[PIPE_BUF];


int main()
{
    int fd;
    FILE *fp;
    pid_t k;
    int status;
    int len, id;

    fp = fopen("serverfifo.dat", "w");
    fprintf(fp, SERVER_NAME);
    printf("serverfifo.dat file has been created by server\n");
    fclose(fp);

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo(SERVER_NAME, 0666);

    char arr1[PIPE_BUF];
    while (1)
    {

        // Open FIFO for Read only
        fprintf(stdout,"[%d]$ ",getpid());
        fd = open(SERVER_NAME, O_RDONLY);

        // Read from FIFO
        read(fd, arr1, sizeof(arr1));
        close(fd);


        len = strlen(arr1);

        if(len==1){
            fprintf(stderr, "%s\n", "server can't process the request");
            continue;
        }

        

        arr1[len-1]='\0';
        
        for(id=0;token[id]!=NULL;id++)
            token[id]=NULL;

        for(id=0;req[id]!=NULL;id++)
            req[id]=NULL;

        parseRequest(arr1);


        // Print the read message
        printf("From Client [%s], Request Item : %s\n", req[0],req[1]);

        parse(req[1]);


        char client[80];
        strcpy(client,"client");
        strcat(client,req[0]);

        printf("\nclient fifo: %s\n",client );

        k = fork();
        if(k==0){

            // child code
            mkfifo(client, 0666);
            int file_desc = open(client,O_WRONLY);

            

            
            // here the newfd is the file descriptor of stdout (i.e. 1)
            dup2(file_desc,1) ; 
            close(file_desc);
            if(execvp(req[1],token) == -1)
            {
                fprintf(stderr, "%s\n", "server can't process the request");
                close(file_desc);
                exit(1);
            }


        }
        else{
            //parent code
            waitpid(k, &status, 0);
        }


        
    }
    return 0;
}