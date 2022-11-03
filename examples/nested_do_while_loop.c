#include <stdio.h>

int a[1000];

int main() {
    int n;
    scanf("%d", &n);
    int i = 1;
    do {
        int j = 2;
        do {
            a[n + j - i] = a[j];
            // printf("%d, %d\n", i, j);
            ++ j;
        } while (j <= n);
        ++ i;
    } while (i <= n);
    return 0;
}