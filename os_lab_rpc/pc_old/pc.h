/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _PC_H_RPCGEN
#define _PC_H_RPCGEN

#include <rpc/rpc.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

int pc_init();

#define PC_PROG 0x20000001
#define PC_VER 1

#if defined(__STDC__) || defined(__cplusplus)
#define consumer 1
extern  enum clnt_stat consumer_1(void *, char *, CLIENT *);
extern  bool_t consumer_1_svc(void *, char *, struct svc_req *);
#define producer 2
extern  enum clnt_stat producer_1(void *, char *, CLIENT *);
extern  bool_t producer_1_svc(void *, char *, struct svc_req *);
extern int pc_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define consumer 1
extern  enum clnt_stat consumer_1();
extern  bool_t consumer_1_svc();
#define producer 2
extern  enum clnt_stat producer_1();
extern  bool_t producer_1_svc();
extern int pc_prog_1_freeresult ();
#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_PC_H_RPCGEN */
