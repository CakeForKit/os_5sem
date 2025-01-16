#include "pc.h"
#include <unistd.h>

void
producer_consumer_program_1(char *host, char type)
{
    CLIENT *clnt;
    enum clnt_stat retval_1;
    char result_1;
    int  produce_1_arg;
    enum clnt_stat retval_2;
    char result_2;
    int  consume_1_arg;
    clnt = clnt_create(host, PRODUCER_CONSUMER_PROGRAM, PRODUCER_CONSUMER_VERSION, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }
    srand(time(NULL));
    if (type == 'p') {
        produce_1_arg = getpid();
        while (1) {
            sleep(1 + rand() % 4);
            retval_1 = produce_1((void*)&produce_1_arg, &result_1, clnt);
            if (retval_1 != RPC_SUCCESS) {
                clnt_perror(clnt, "produce call failed");
            }
            printf("[pid=%d] producer: %c\n", produce_1_arg, result_1);
        }
    }
    if (type == 'c') {
        consume_1_arg = getpid();
        while (1) {
            sleep(1 + rand() % 4);
            retval_2 = consume_1((void*)&consume_1_arg, &result_2, clnt);
            if (retval_2 != RPC_SUCCESS) {
                clnt_perror(clnt, "consume call failed");
            }
            printf("[pid=%d] consumer: %c\n", consume_1_arg, result_2);
        }
    }
    clnt_destroy(clnt);
}

int
main (int argc, char *argv[])
{
    char *host;
    char type;
    if (argc != 3) {
      printf ("usage: %s server_host type\n", argv[0]);
      exit(1);
    }
    type = *argv[2];
    if (type != 'p' && type != 'c') {
      printf ("type must be one of 'p' or 'c'\n");
      exit(1);
    }
    host = argv[1];
    producer_consumer_program_1(host, type);
    exit(0);
}
