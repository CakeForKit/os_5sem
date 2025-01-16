
program PC_PROG
{
    version PC_VER
    {
        char PRODUCER(int pid) = 1;
        char CONSUMER(int pid) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001;
