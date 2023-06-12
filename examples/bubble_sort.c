#include <stdio.h>

int n, a[100];

void swap(int *x, int *y) {
    int t = *x;
    *x = *y;
    *y = t;
}

int main() {
    scanf("%d", &n);
    for (int i = 0; i < n; ++i) {
        scanf("%d", &a[i]);
    }
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (a[i] > a[j]) {
                swap(&a[i], &a[j]);
            }
        }
    }
    for (int i = 0; i < n; ++i) {
        printf("%d\n", a[i]);
    }
    return 0;
}
