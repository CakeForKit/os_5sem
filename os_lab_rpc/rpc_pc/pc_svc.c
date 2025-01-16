#include "pc.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

#define P -1
#define V  1

#define SB 0
#define SE 1
#define SF 2

struct sembuf start_produce[2] = { {SE, P, 0}, {SB, P, 0} };
struct sembuf stop_produce[2] =  { {SB, V, 0}, {SF, V, 0} };
struct sembuf start_consume[2] = { {SF, P, 0}, {SB, P, 0} };
struct sembuf stop_consume[2] =  { {SB, V, 0}, {SE, V, 0} };
int semid;

pthread_t p_thread;
pthread_attr_t attr;

void *
serv_request(void *data)
{
    struct thr_data
    {
        struct svc_req *rqstp;
        SVCXPRT *transp;
    } *ptr_data;
    union {
        int produce_1_arg;
        int consume_1_arg;
    } argument;
    union {
        char produce_1_res;
        char consume_1_res;
    } result;
    bool_t retval;
    xdrproc_t _xdr_argument, _xdr_result;
    bool_t(*local)(char *, void *, struct svc_req *);
    ptr_data = (struct thr_data  *)data;
    struct svc_req *rqstp = ptr_data->rqstp;
    register SVCXPRT *transp = ptr_data->transp;
    switch(rqstp->rq_proc) {
    case NULLPROC:
        (void) svc_sendreply(transp, (xdrproc_t) xdr_void, (char *)NULL);
        return;
    case PRODUCE:
        _xdr_argument = (xdrproc_t) xdr_int;
        _xdr_result = (xdrproc_t) xdr_char;
        local = (bool_t(*)(char *, void *,  struct svc_req *))produce_1_svc;
        break;
    case CONSUME:
        _xdr_argument = (xdrproc_t) xdr_int;
        _xdr_result = (xdrproc_t) xdr_char;
        local = (bool_t(*)(char *, void *,  struct svc_req *))consume_1_svc;
        break;
    default:
        svcerr_noproc(transp);
        return;
    }
    memset((char *)&argument, 0, sizeof(argument));
    if (!svc_getargs(transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
        svcerr_decode(transp);
        return;
    }
    if (rqstp->rq_proc == PRODUCE) {
        if (semop(semid, start_produce, 2) == -1)
        {
            perror("start produce semop");
            exit(1);
        }
        retval = (bool_t) (*local)((char *)&argument, (void *)&result, rqstp);
        if (semop(semid, stop_produce, 2) == -1)
        {
            perror("stop produce semop");
            exit(1);
        }
    } else {
        if (semop(semid, start_consume, 2) == -1) {
            perror("start consume semop");
            exit(1);
        }
        retval = (bool_t) (*local)((char *)&argument, (void *)&result, rqstp);
        if (semop(semid, stop_consume, 2) == -1)
        {
            perror("stop consume semop");
            exit(1);
        }
    }
    if (retval > 0 && !svc_sendreply(transp, (xdrproc_t) _xdr_result, (char *)&result)) {
        svcerr_systemerr(transp);
    }
    if (!svc_freeargs(transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
        fprintf(stderr, "%s", "unable to free arguments");
        exit(1);
    }
    if (!producer_consumer_program_1_freeresult(transp, _xdr_result, (caddr_t) &result))
        fprintf(stderr, "%s", "unable to free results");
    return;
}

static void
producer_consumer_program_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
    struct data_str
    {
        struct svc_req *rqstp;
        SVCXPRT *transp;
    } *data_ptr = (struct data_str*)malloc(sizeof(struct data_str));
    data_ptr->rqstp = rqstp;
    data_ptr->transp = transp;
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&p_thread,&attr,serv_request,(void *)data_ptr);
}

int
main(int argc, char **argv)
{
    register SVCXPRT *transp;
    semid = semget(IPC_PRIVATE, 3, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
    if (semid == -1)
    {
        perror("semget");
        exit(1);
    }
    if (semctl(semid, SB, SETVAL, 1) == -1)
    {
        perror("bin semctl");
        exit(1);
    }
    if (semctl(semid, SE, SETVAL, BUFF_SIZE) == -1)
    {
        perror("empty semctl");
        exit(1);
    }
    if (semctl(semid, SF, SETVAL, 0) == -1)
    {
        perror("full semctl");
        exit(1);
    }
    pmap_unset(PRODUCER_CONSUMER_PROGRAM, PRODUCER_CONSUMER_VERSION);
    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
	    fprintf (stderr, "%s", "cannot create udp service.");
	    exit(1);
    }
    if (!svc_register(transp, PRODUCER_CONSUMER_PROGRAM, PRODUCER_CONSUMER_VERSION, producer_consumer_program_1, IPPROTO_UDP)) {
	    fprintf (stderr, "%s", "unable to register (PRODUCER_CONSUMER_PROGRAM, PRODUCER_CONSUMER_VERSION, udp).");
	    exit(1);
    }
    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (transp == NULL) {
	    fprintf (stderr, "%s", "cannot create tcp service.");
	    exit(1);
    }
    if (!svc_register(transp, PRODUCER_CONSUMER_PROGRAM, PRODUCER_CONSUMER_VERSION, producer_consumer_program_1, IPPROTO_TCP)) {
	    fprintf (stderr, "%s", "unable to register (PRODUCER_CONSUMER_PROGRAM, PRODUCER_CONSUMER_VERSION, tcp).");
	    exit(1);
    }
    svc_run();
    fprintf (stderr, "%s", "svc_run returned");
    exit(1);
}
