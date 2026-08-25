#include <stdio.h>

char dois()
{
    return '2';
}

int soma(int a, int b) {
    return a + b;
}

int main()
{
    printf("%d", soma(3, 2));
    return 0;
}