#include <stdio.h>

int a[1000];
int n;
// char str[20];

int main() {
    scanf("%d", &n);
    int sum = 0;
    n += 6;
    int m = n << 1;
    for (int i = 7; i < m; ++ i) {
        a[i] = a[i - 1] + i;
    }
    for (int i = 6; i < m; ++i) {
        sum = sum + a[i];
    }
    return 0;
}
