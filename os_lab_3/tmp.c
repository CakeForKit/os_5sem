#include <signal.h>

#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stddef.h>		/* for offsetof */
#include <string.h>		/* for convenience */
#include <unistd.h>		/* for convenience */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

void handler_SIGUSR2(int sig_numb)
{
    printf("SIGUSR2\n");
    while(1);
}

void handler_SIGUSR1(int sig_numb)
{
    printf("SIGUSR1\n");
    exit(EXIT_SUCCESS);
}


int main(void) {
    struct sigaction sa, sa1;

    sa.sa_handler = handler_SIGUSR2;
    sigemptyset(&sa.sa_mask);
    // sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR2, &sa, NULL) == -1) 
        perror("Error sigaction SIGHUP\n");

    sa1.sa_handler = handler_SIGUSR1;
    sigfillset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa1, NULL) == -1) 
        perror("Error sigaction SIGUSR1\n");
    
    while(1);
    return 0;
}