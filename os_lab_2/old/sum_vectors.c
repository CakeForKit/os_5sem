#include <stdio.h>
#include <stdlib.h>

int main()
{
    int n;

    printf("sv: Введите размерность векторов: ");
    if (scanf("%d", &n) != 1)
    {
        printf("sv: Надо ввести число.\n");
        return 1;
    }
    if (n <= 0)
    {
        printf("sv: Размерность векторов должна быть больше нуля.\n");
        return 1; 
    }

    printf("sv: Размерность векторов: = 1x%d\n", n);

    int *vector1 = malloc(n * sizeof(int));
    if (!vector1)
    {
        printf("sv: Ошибка выделения памяти.\n");
        return 1;
    }
    int *vector2 = malloc(n * sizeof(int));
    if (!vector2)
    {
        printf("sv: Ошибка выделения памяти.\n");
        return 1;
    }

    printf("sv: Введите элементы первого вектора:\n");
    for (int i = 0; i < n; ++i)
    {
        printf("sv: %d-й элемент 1-го вектора: ", i);
        if (scanf("%d", &vector1[i]) != 1)
        {
            printf("sv: Надо ввести число.\n");
            return 1;
        }
    }

    printf("sv: Введите элементы второго вектора:\n");
    for (int i = 0; i < n; ++i)
    {
        printf("sv: %d-й элемент 2-го вектора: ", i);
        if (scanf("%d", &vector2[i]) != 1)
        {
            printf("sv: Надо ввести число.\n");
            return 1;
        }
    }

    int *vector_res = malloc(n * sizeof(int));
    if (!vector_res)
    {
        printf("sv: Ошибка выделения памяти.\n");
        return 1;
    }
    for (int i = 0; i < n; ++i)
    {
        vector_res[i] = vector1[i] + vector2[i];
    }

    printf("sv: Вектор суммы: [ ");
    for (int i = 0; i < n; ++i)
    {
        printf("%d ", vector_res[i]);
    }
    printf("]\n");

    free(vector1);
    free(vector2);
    free(vector_res);

    return 0; 
}