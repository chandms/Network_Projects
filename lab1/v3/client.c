// C program to implement one side of FIFO
// This side reads first, then reads
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PIPE_BUF 512

int main()
{
    FILE *fp;
    char buff[255];
    int cpid;
    char* num;


    char *fname = "serverfifo.dat";

    if( access( fname, F_OK ) == 0 ) {
        fp = fopen( fname, "r");
        fscanf(fp, "%s", buff);
        printf("fifo name : %s\n", buff );
        int fd1;

        
        // Creating the named file(FIFO)
        // mkfifo(<pathname>,<permission>)
        mkfifo(buff, 0666);

        while (1)
        {
            char str[PIPE_BUF], str2[PIPE_BUF];
            cpid = getpid();

            char req[PIPE_BUF];

            // Now open in write mode and write
            // string taken from user.
            
            fgets(str, sizeof str, stdin);
            if(str[strlen(str)-1] == '\n'){
                asprintf(&num, "%d", cpid);
                strcat(strcpy(req,num),"\n");
                strcat(req,str);

                fd1 = open(buff,O_WRONLY);
                write(fd1, req, strlen(req)+1);
                close(fd1);

                char client[80];
                strcpy(client,"client");
                strcat(client,num);

                mkfifo(client, 0666);
                int file_desc = open(client,O_RDONLY);
          
                read(file_desc, str2, sizeof(str2));
                printf("Result : %s\n", str2);
                close(file_desc);
            }
            else{
                fprintf(stderr,"%s","Processing limit crosses\n");

            }
        }
    }
    else{
        fprintf(stderr,"%s","serverfifo.dat file does not exist \n");
    }
    return 0;
}