#include "pc.h"
#include <stdbool.h>

char buffer[BUFF_SIZE];
char *producer_ptr = buffer;
char *consumer_ptr = buffer;
char alpha = 'a';

bool_t
produce_1_svc(int *argp, char *result, struct svc_req *rqstp)
{
    *result = *producer_ptr = alpha;
    producer_ptr++;
    alpha = alpha < 'z' ? alpha + 1 : 'a';
    printf("[pid=%d] producer: %c\n", *argp, *result);
    return true;
}

bool_t
consume_1_svc(int *argp, char *result, struct svc_req *rqstp)
{
    *result = *consumer_ptr;
    consumer_ptr++;
    printf("[pid=%d] consumer: %c\n", *argp, *result);
    return true;
}

int
producer_consumer_program_1_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
    xdr_free(xdr_result, result);
    return 1;
}
