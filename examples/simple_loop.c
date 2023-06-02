#include <stdio.h>

int N;

int main() {
    int a[100];
    scanf("%d", &N);
    for (int i = 0; i < N; ++i) {
        a[i] = i;
    }
    for (int i = 0; i < N; ++i) {
        printf("%d\n", a[i]);
    }
    return 0;
}
