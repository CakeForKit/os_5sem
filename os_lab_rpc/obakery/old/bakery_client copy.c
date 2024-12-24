/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "bakery.h"


void
bakery_prog_1(char *host, int index)
{
	CLIENT *clnt;
	struct BAKERY  *result_1;
	struct BAKERY  request;
	struct BAKERY  *result_2;

#ifndef	DEBUG
	clnt = clnt_create (host, BAKERY_PROG, BAKERY_VER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	srand(getpid());
	request.ind = index;
	request.pid = getpid();
	request.op = rand() % 4;
	request.arg1 = rand() % 10;
	request.arg2 = rand() % 10;

	result_1 = get_number_1(&request, clnt);
	if (result_1 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	request.num = result_1->num;
	// printf("PID=%d num=%d\n", request.pid, result_1->num);
	result_2 = serve_1(&request, clnt);
	if (result_2 == (struct BAKERY *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	char *op;
	switch(request.op)
	{
		case ADD:
			op = "add";
			break;
		case SUB:
			op = "sub";
			break;
		case MUL:
			op = "mul";
			break;
		case DIV:
			op = "div";
			break;
		default:
			break;
	}
	printf("PID=%d num=%d, %lf (%s) %lf = %lf\n", request.pid, result_1->num, request.arg1, op, request.arg2, result_2->result);
	
#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}


int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	pid_t client_pid[COUNT_CLIENTS];

	for (size_t i = 0; i < COUNT_CLIENTS; i++)
	{
		if ((client_pid[i] = fork()) == -1)
		{
			perror("Can't fork\n");
			exit(1);
		}
		else if (client_pid[i] == 0)
		{
			bakery_prog_1(host, i);
			return 0;
		}
	}
	printf("created: ");
	for (size_t i = 0; i < COUNT_CLIENTS; i++)
		printf("%d ", client_pid[i]);
	printf("\n");

	int wstatus;
	pid_t w;
	for (size_t i = 0; i < COUNT_CLIENTS; i++)
	{
		w = waitpid(-1, &wstatus, 0); // until one of its children terminates (ANY child process.)
        if (w == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: waitpid PID=%d, errno=%d", getpid(), errno);
            perror(err_msg);
			exit(EXIT_FAILURE);
        }

        printf("PID=%d ", w);
        if (WIFEXITED(wstatus)) {                                    
            printf("exited, status = %d ", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d ", WTERMSIG(wstatus));
        }
        printf("\n");
	}
exit (0);
}
