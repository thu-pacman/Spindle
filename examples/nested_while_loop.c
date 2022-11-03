#include <stdio.h>

int a[1000];

int main() {
    int n;
    scanf("%d", &n);
    int i = 1;
    while (i < n) {
        int j = 2;
        while (j < n) {
            a[n + j - i] = a[j];
            // printf("%d, %d\n", i, j);
            ++ j;
        }
        ++ i;
    }
    return 0;
}