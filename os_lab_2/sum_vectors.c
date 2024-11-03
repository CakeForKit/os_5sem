#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int n;

    printf("PID %d: Введите размерность векторов: \n", getpid());
    if (scanf("%d", &n) != 1)
    {
        printf("PID %d: Надо ввести число.\n", getpid());
        return 1;
    }
    if (n <= 0)
    {
        printf("PID %d: Размерность векторов должна быть больше нуля.\n", getpid());
        return 1; 
    }
    printf("\t\t\t\t\t\t\tPID %d: n = %d\n", getpid(), n);

    int *vector1 = malloc(n * sizeof(int));
    if (!vector1)
    {
        printf("PID %d: Ошибка выделения памяти.\n", getpid());
        return 1;
    }
    int *vector2 = malloc(n * sizeof(int));
    if (!vector2)
    {
        printf("PID %d: Ошибка выделения памяти.\n", getpid());
        return 1;
    }

    for (int i = 0; i < n; ++i)
    {
        printf("PID %d: Введите %d-й элемент 1-го вектора: ", getpid(), i+1);
        if (scanf("%d", &vector1[i]) != 1)
        {
            printf("PID %d: Надо ввести число.\n", getpid());
            return 1;
        }
        printf("\t\t\t\t\t\t\tPID %d: Введен vector1[%d] = %d\n", getpid(), i, vector1[i]);
    }
    
    for (int i = 0; i < n; ++i)
    {
        printf("PID %d: Введите %d-й элемент 2-го вектора: ", getpid(), i+1);
        if (scanf("%d", &vector2[i]) != 1)
        {
            printf("PID %d: Надо ввести число.\n", getpid());
            return 1;
        }
        printf("\t\t\t\t\t\t\tPID %d: Введен vector2[%d] = %d\n", getpid(), i, vector2[i]);
    }

    int *vector_res = malloc(n * sizeof(int));
    if (!vector_res)
    {
        printf("PID %d: Ошибка выделения памяти.\n", getpid());
        return 1;
    }
    for (int i = 0; i < n; ++i)
    {
        vector_res[i] = vector1[i] + vector2[i];
    }

    printf("PID %d: Вектор суммы: [ ", getpid());
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