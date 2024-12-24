const ADD = 0;
const SUB = 1;
const MUL = 2;
const DIV = 3;

struct BAKERY
{
    int num;
    int op;
    double arg1;
    double arg2;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        int GET_NUMBER(void) = 1; 
        double SERVE(struct BAKERY) = 2;
    } = 1; /* Version number = 1 */
} = 0x20000001;