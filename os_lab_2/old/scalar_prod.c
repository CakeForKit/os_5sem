#include <stdio.h>
#include <stdlib.h>

int main()
{
    int n;

    printf("sp: Введите размерность векторов: ");
    if (scanf("%d", &n) != 1)
    {
        printf("sp: Надо ввести число.\n");
        return 1;
    }
    if (n <= 0)
    {
        printf("sp: Размерность векторов должна быть больше нуля.\n");
        return 1; 
    }

    printf("sp: Размерность векторов: = 1x%d\n", n);

    int *vector1 = malloc(n * sizeof(int));
    if (!vector1)
    {
        printf("sp: Ошибка выделения памяти.\n");
        return 1;
    }
    int *vector2 = malloc(n * sizeof(int));
    if (!vector2)
    {
        printf("sp: Ошибка выделения памяти.\n");
        return 1;
    }

    printf("sp: Введите элементы первого вектора:\n");
    for (int i = 0; i < n; ++i)
    {
        printf("sp: %d-й элемент 1-го вектора: ", i);
        if (scanf("%d", &vector1[i]) != 1)
        {
            printf("sp: Надо ввести число.\n");
            return 1;
        }
    }

    printf("sp: Введите элементы второго вектора:\n");
    for (int i = 0; i < n; ++i)
    {
        printf("sp: %d-й элемент 2-го вектора: ", i);
        if (scanf("%d", &vector2[i]) != 1)
        {
            printf("sp: Надо ввести число.\n");
            return 1;
        }
    }

    double scalarProduct = 0.0;
    for (int i = 0; i < n; ++i)
    {
        scalarProduct += vector1[i] * vector2[i];
    }

    printf("sp: Скалярное произведение векторов: %.2lf\n", scalarProduct);

    free(vector1);
    free(vector2);

    return 0; 
}