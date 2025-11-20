#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    int pipeFds[2];
    pid_t pid;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (pipe(pipeFds) == -1)
    {
        perror("pipe failed");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid == -1)
    {
        perror("fork failed");
        close(pipeFds[0]);
        close(pipeFds[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        /* Child process: write message to the pipe */
        size_t msgLen = strlen(argv[1]) + 1; /* include terminating null */

        close(pipeFds[0]); /* Close unused read end */

        if (write(pipeFds[1], argv[1], msgLen) == -1)
        {
            perror("write failed");
            close(pipeFds[1]);
            _exit(EXIT_FAILURE);
        }

        close(pipeFds[1]);
        _exit(EXIT_SUCCESS);
    }
    else
    {
        /* Parent process: read message from the pipe */
        char buffer[4096];
        ssize_t bytesRead;

        close(pipeFds[1]); /* Close unused write end */

        bytesRead = read(pipeFds[0], buffer, sizeof(buffer) - 1);
        if (bytesRead == -1)
        {
            perror("read failed");
            close(pipeFds[0]);
            return EXIT_FAILURE;
        }

        buffer[bytesRead] = '\0';
        printf("%s\n", buffer);

        close(pipeFds[0]);
    }

    return EXIT_SUCCESS;
}

