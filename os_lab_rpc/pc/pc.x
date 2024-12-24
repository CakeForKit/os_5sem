
program PC_PROG
{
    version PC_VER
    {
        char consumer(void) = 1;
        char producer(void) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001; /* RPC program number */