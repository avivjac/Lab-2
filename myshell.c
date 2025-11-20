#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <signal.h>
#include "LineParser.h"

#define MAX_LINE_SIZE 2048

/* Global variable for debug mode */
static int debugMode = 0;

/* Handle built-in cd command */
static int handleCd(cmdLine *pCmdLine)
{
    if (pCmdLine->argCount == 0)
        return 0;

    if (strcmp(pCmdLine->arguments[0], "cd") != 0)
        return 0;

    if (pCmdLine->argCount < 2)
    {
        fprintf(stderr, "cd: missing operand\n");
    }
    else if (chdir(pCmdLine->arguments[1]) == -1)
    {
        perror("cd failed");
    }

    return 1;
}

/* Handle signal management commands */
static int handleSignalCommand(cmdLine *pCmdLine)
{
    int signalToSend;

    if (pCmdLine->argCount == 0)
    {
        return 0;
    }

    if (strcmp(pCmdLine->arguments[0], "zzzz") == 0)
    {
        signalToSend = SIGSTOP;
    }
    else if (strcmp(pCmdLine->arguments[0], "kuku") == 0)
    {
        signalToSend = SIGCONT;
    }
    else if (strcmp(pCmdLine->arguments[0], "blast") == 0)
    {
        signalToSend = SIGINT;
    }
    else
    {
        return 0;
    }

    if (pCmdLine->argCount < 2)
    {
        fprintf(stderr, "%s: missing process id\n", pCmdLine->arguments[0]);
        return 1;
    }

    char *endPtr = NULL;
    long pidValue = strtol(pCmdLine->arguments[1], &endPtr, 10);

    if (endPtr == pCmdLine->arguments[1] || *endPtr != '\0' || pidValue <= 0)
    {
        fprintf(stderr, "%s: invalid process id '%s'\n", pCmdLine->arguments[0], pCmdLine->arguments[1]);
        return 1;
    }

    if (kill((pid_t)pidValue, signalToSend) == -1)
    {
        perror("kill failed");
    }

    return 1;
}

/* Function to execute a command */
void execute(cmdLine *pCmdLine)
{
    pid_t pid;
    
    /* Fork a child process */
    pid = fork();
    
    if (pid == -1)
    {
        /* Fork failed - this is a serious problem */
        perror("fork failed");
        return;
    }
    
    if (pid == 0)
    {
        /* Child process */
        int inputFd = -1;
        int outputFd = -1;

        /* Handle input redirection */
        if (pCmdLine->inputRedirect != NULL)
        {
            inputFd = open(pCmdLine->inputRedirect, O_RDONLY);
            if (inputFd == -1)
            {
                perror("open input failed");
                _exit(EXIT_FAILURE);
            }
            if (dup2(inputFd, STDIN_FILENO) == -1)
            {
                perror("dup2 input failed");
                close(inputFd);
                _exit(EXIT_FAILURE);
            }
            close(inputFd);
        }

        /* Handle output redirection */
        if (pCmdLine->outputRedirect != NULL)
        {
            outputFd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outputFd == -1)
            {
                perror("open output failed");
                _exit(EXIT_FAILURE);
            }
            if (dup2(outputFd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output failed");
                close(outputFd);
                _exit(EXIT_FAILURE);
            }
            close(outputFd);
        }

        /* Execute the command */
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
        {
            perror("execvp failed");
            /* Use _exit() instead of exit() in child process */
            /* _exit() doesn't flush stdio buffers or call atexit handlers */
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        /* Parent process */
        if (debugMode)
        {
            fprintf(stderr, "PID: %d\n", pid);
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        
        /* Wait for child process to complete (for blocking commands) */
        if (pCmdLine->blocking)
        {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

int main(int argc, char **argv)
{
    char cwd[PATH_MAX];  /* Buffer to hold current working directory */
    char line[MAX_LINE_SIZE];  /* Buffer to hold user input */
    cmdLine *parsedLine;  /* Pointer to parsed command line structure */
    
    /* Check for debug flag */
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        debugMode = 1;
    }
    
    /* Infinite loop for the shell */
    while (1)
    {
        /* Step 1: Display prompt with current working directory */
        if (getcwd(cwd, PATH_MAX) == NULL)
        {
            perror("getcwd failed");
            exit(EXIT_FAILURE);
        }
        printf("%s$ ", cwd);
        
        /* Step 2: Read a line from stdin */
        if (fgets(line, MAX_LINE_SIZE, stdin) == NULL)
        {
            /* EOF (Ctrl+D) or error */
            printf("\n");
            break;
        }
        
        /* Remove newline character if present */
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        
        /* Step 3: Check for "quit" command */
        if (strcmp(line, "quit") == 0)
        {
            break;
        }
        
        /* Step 4: Parse the input line */
        parsedLine = parseCmdLines(line);
        
        if (parsedLine == NULL)
        {
            /* Empty line or parsing failed, continue to next iteration */
            continue;
        }
        
        /* Step 5: Execute the command */
        if (!handleCd(parsedLine) && !handleSignalCommand(parsedLine))
        {
            execute(parsedLine);
        }
        
        /* Step 6: Release resources (parent process only) */
        freeCmdLines(parsedLine);
    }
    
    return 0;
}

