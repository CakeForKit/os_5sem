/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "pc.h"

char buffer[SIZE_BUF];
char *prod_ptr = buffer;
char *cons_ptr = buffer;
char alpha = 'a';

bool_t
producer_1_svc(int *argp, char *result, struct svc_req *rqstp)
{
	*prod_ptr = alpha;
	*result = alpha;
	printf("Производитель PID=%d положил '%c'\n", *argp, *result);
    prod_ptr++;
	if (alpha == 'z')
		alpha = 'a';
	else
		alpha++;
	return TRUE;
}

bool_t
consumer_1_svc(int *argp, char *result, struct svc_req *rqstp)
{
	*result = *cons_ptr;
    cons_ptr++;
	printf("Потребитель   PID=%d взял    '%c'\n", *argp, *result);
	return TRUE;
}

int
pc_prog_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
	xdr_free (xdr_result, result);

	/*
	 * Insert additional freeing code here, if needed
	 */

	return 1;
}
