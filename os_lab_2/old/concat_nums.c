#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int a, b, res;

    printf("%d: Введите неотрицательное первое число: ", getpid());
    if (scanf("%d", &a) != 1)
    {
        printf("%d: Надо ввести число.\n", getpid());
        return 1;
    }
    if (a < 0) {
        printf("%d: Надо ввести число >= 0\n", getpid());
        return 1;
    }

    printf("%d: Введите неотрицательное второе число: ", getpid());
    if ((res = scanf("%d", &b)) != 1)
    {
        printf("%d: Надо ввести число.\n", getpid());
        return 1;
    } 
    printf("----%d", res);
    if (b < 0) {
        printf("%d: Надо ввести число >= 0\n", getpid());
        return 1;
    }

    int p = 10;
    while (b >= 10)
        p *= 10;
    res = a * p + b;
    printf("%d: Конкатенация чисел: %d\n", getpid(), res);

    return 0; 
}