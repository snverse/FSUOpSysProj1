#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char *argv) {
    char * args[] = { "echo", "arg1", "arg2", NULL };
    printf("%s\n", *args);
    
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if (pid == 0)
    {
        // child
        printf("child\n");
        if(execv("/bin/echo\0", (char* const*)args) == -1)
        {
            perror("error\n");
        }
        printf("exit child\n");
        exit(-1);
    }
    else if (pid < 0)
    {
        perror("error\n");
    }
    else
    {
        do {
            // parent
            printf("parent\n");
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    

    printf("Exiting program\n");
    return 0;
}
