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

#define PIPE_BUF 512

char* token[PIPE_BUF];
void  parse(char buf[PIPE_BUF]){
    char s[2]=" ";

    int i=0;
    token[0]=strtok(buf,s);

    while(token[i]!=NULL){
       // printf(" %s\n",token[i]);
        i++;
        token[i]=strtok(NULL, s);
    }
    
    //printf("done\n");
}

int main()
{
    int fd;
    FILE *fp;
    pid_t k;
    int status;
    int len;

    fp = fopen("serverfifo.dat", "w");
    fprintf(fp, "myfifo");
    printf("serverfifo.dat file has been created by server\n");
    fclose(fp);

    // Creating the named file(FIFO)
    // mkfifo(<pathname>, <permission>)
    mkfifo("myfifo", 0666);

    char arr1[PIPE_BUF];
    while (1)
    {

        // Open FIFO for Read only
        fd = open("myfifo", O_RDONLY);

        // Read from FIFO
        read(fd, arr1, sizeof(arr1));

        // Print the read message
        printf("Request Item : %s\n", arr1);
        len = strlen(arr1);

        if(len==1){
            fprintf(stderr, "%s\n", "server can't process the request");
            continue;
        }


        arr1[len-1]='\0';
        parse(arr1);

        k = fork();
        if(k==0){
            // child code
            if(execvp(arr1,token) == -1)
            {
                fprintf(stderr, "%s\n", "server can't process the request");
                exit(1);
            }

        }
        else{
            //parent code
            waitpid(k, &status, 0);
        }


        close(fd);
    }
    return 0;
}