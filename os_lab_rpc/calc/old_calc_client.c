/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "calculator.h"


void
calculator_prog_1(char *host)
{
	CLIENT *clnt;
	struct CALCULATOR  *result_1;
	struct CALCULATOR  calculator_proc_1_arg;

#ifndef	DEBUG
	clnt = clnt_create (host, CALCULATOR_PROG, CALCULATOR_VER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

	result_1 = calculator_proc_1(&calculator_proc_1_arg, clnt);
	if (result_1 == (struct CALCULATOR *) NULL) {
		clnt_perror (clnt, "call failed");
	}
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
	calculator_prog_1 (host);
exit (0);
}
