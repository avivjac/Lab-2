#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>

void handler(int sig)
{
	/* Print the signal name with a message */
	printf("\nReceived Signal: %s\n", strsignal(sig));	
	/* Set the default handler for this signal */
	signal(sig, SIG_DFL);
	
	/* Before propagating, reinstate the complementary handler */
	/* After handling SIGTSTP, reinstate custom handler for SIGCONT */
	if (sig == SIGTSTP)
	{
		signal(SIGCONT, handler);
	}
	/* After handling SIGCONT, reinstate custom handler for SIGTSTP */
	else if (sig == SIGCONT)
	{
		signal(SIGTSTP, handler);
	}
	
	/* Propagate the signal to the default handler */
	raise(sig);
}

int main(int argc, char **argv)
{

	printf("Starting the program\n");
	signal(SIGINT, handler);
	signal(SIGTSTP, handler);
	signal(SIGCONT, handler);

	while (1)
	{
		sleep(1);
	}

	return 0;
}