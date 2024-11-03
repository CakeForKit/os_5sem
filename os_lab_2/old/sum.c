#include <stdio.h>
#include <stdlib.h>

int main()
{
    int a, b;

    printf("s: Введите первое число: ");
    if (scanf("%d", &a) != 1)
    {
        printf("s: Надо ввести число.\n");
        return 1;
    }

    printf("s: Введите второе число: ");
    if (scanf("%d", &b) != 1)
    {
        printf("s: Надо ввести число.\n");
        return 1;
    }

    printf("s: Сумма чисел: %d\n", a + b);

    return 0; 
}