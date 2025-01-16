/*
 * filename: pc.x
     * function: Define constants, non-standard data types and the calling process in remote calls
 */

program PRODUCER_CONSUMER_PROGRAM
{
    version PRODUCER_CONSUMER_VERSION
    {
        char PRODUCE(int id) = 1;
        char CONSUME(int id) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001; /* RPC program number */
