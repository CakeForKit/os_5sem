
const ADD = 0;
const SUB = 1;
const MUL = 2;
const DIV = 3;

struct BAKERY
{
    int num;
    int pid;
    int ind;
    int op;
    float arg1;
    float arg2;
    int result;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        struct BAKERY GET_NUMBER(struct BAKERY) = 1; 
        struct BAKERY SERVE(struct BAKERY) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001;