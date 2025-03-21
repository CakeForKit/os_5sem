/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "pc.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif
pthread_t p_thread;
pthread_attr_t attr;

void *serv_request(void *data) {
	struct thr_data
    {
        struct svc_req *rqstp;
        SVCXPRT *transp;
		int arg;
    } *ptr_data;
    union {
        int producer_1_arg;
        int consumer_1_arg;
    } argument;
    union {
        char producer_1_res;
        char consumer_1_res;
    } result;
	bool_t retval;
    xdrproc_t _xdr_argument, _xdr_result;
	(void) _xdr_argument;
    bool_t(*local)(int *, char *, struct svc_req *);
    ptr_data = (struct thr_data  *)data;
    struct svc_req *rqstp = ptr_data->rqstp;
    register SVCXPRT *transp = ptr_data->transp;
	argument.producer_1_arg = ptr_data->arg;
	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return NULL;
	case PRODUCER:
		// _xdr_argument = (xdrproc_t) xdr_int;
		_xdr_result = (xdrproc_t) xdr_char;
		local = (bool_t (*) (int *, char *,  struct svc_req *))producer_1_svc;
		break;
	case CONSUMER:
		// _xdr_argument = (xdrproc_t) xdr_int;
		_xdr_result = (xdrproc_t) xdr_char;
		local = (bool_t (*) (int *, char *,  struct svc_req *))consumer_1_svc;
		break;
	default:
		svcerr_noproc (transp);
		return NULL;
	}
	// memset ((char *)&argument, 0, sizeof (argument));
	// if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
	// 	svcerr_decode (transp);
	// 	return NULL;
	// }
	if (rqstp->rq_proc == PRODUCER) {
        retval = (bool_t) (*local)((int *)&argument, (char *)&result, rqstp);
	} else {
        retval = (bool_t) (*local)((int *)&argument, (char *)&result, rqstp);
	}
	if (retval > 0 && !svc_sendreply(transp, (xdrproc_t) _xdr_result, (char *)&result)) {
		svcerr_systemerr (transp);
	}
	// if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
	// 	fprintf (stderr, "%s", "unable to free arguments");
	// 	exit (1);
	// }
	if (!pc_prog_1_freeresult (transp, _xdr_result, (caddr_t) &result))
		fprintf (stderr, "%s", "unable to free results");
	return NULL;
}

static void
pc_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	struct data_str
	{
		struct svc_req *rqstp;
		SVCXPRT *transp;
		int arg;
	} *data_ptr=(struct data_str*)malloc(sizeof(struct data_str));
	data_ptr->rqstp = rqstp;
	data_ptr->transp = transp;

	union {
        int producer_1_arg;
        int consumer_1_arg;
    } argument;
	xdrproc_t _xdr_argument = (xdrproc_t) xdr_int;
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	data_ptr->arg = argument.consumer_1_arg;
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&p_thread, &attr, serv_request, (void *)data_ptr);
}

int
main (int argc, char **argv)
{
	register SVCXPRT *transp;
	int rc = init_sem(argv[0]);
	if (rc == -1) {
		fprintf (stderr, "init_sem");
		exit(1);
	}
	pmap_unset (PC_PROG, PC_VER);
	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, PC_PROG, PC_VER, pc_prog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (PC_PROG, PC_VER, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, PC_PROG, PC_VER, pc_prog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (PC_PROG, PC_VER, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	free_sem();
	exit (1);
	/* NOTREACHED */
}
