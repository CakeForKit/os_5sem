#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a, b, res;

    printf("с: Введите неотрицательное первое число: ");
    if (scanf("%d", &a) != 1)
    {
        printf("с: Надо ввести число.\n");
        return 1;
    } else if (a < 0) {
        printf("с: Надо ввести число >= 0\n");
        return 1;
    }

    printf("с: Введите неотрицательное второе число: ");
    if (scanf("%d", &b) != 1)
    {
        printf("с: Надо ввести число.\n");
        return 1;
    } else if (b < 0) {
        printf("с: Надо ввести число >= 0\n");
        return 1;
    }

    int p = 10;
    while (b >= 10)
        p *= 10;
    res = a * p + b;
    printf("с: Конкатенация чисел: %d\n", res);

    return 0; 
}