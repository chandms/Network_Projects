// C program to implement one side of FIFO
// This side reads first, then reads
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PIPE_BUF 4096

int main()
{
    FILE *fp;
    char buff[255];


    char *fname = "serverfifo.dat";

    if( access( fname, F_OK ) == 0 ) {
        fp = fopen( fname, "r");
        fscanf(fp, "%s", buff);
        printf("fifo name : %s\n", buff );
        int fd1;

        
        // Creating the named file(FIFO)
        // mkfifo(<pathname>,<permission>)
        mkfifo(buff, 0666);

        char str[PIPE_BUF];
        while (1)
        {

            // Now open in write mode and write
            // string taken from user.
            
            fgets(str, sizeof str, stdin);
            if(str[strlen(str)-1] == '\n'){
                fd1 = open(buff,O_WRONLY);
                write(fd1, str, strlen(str)+1);
                close(fd1);
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