#include <stdio.h>

int a[1000];
int n;
char str[20];

int main() {
    scanf("%d", &n);
    int m = n - 1;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            a[n + j - i] = a[j];
        }
    }
    return 0;
}
