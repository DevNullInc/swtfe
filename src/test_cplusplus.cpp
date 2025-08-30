#include <stdio.h>

int main() {
#ifdef __cplusplus
    printf("C++ mode detected\n");
#else
    printf("C mode detected\n");
#endif
    return 0;
}
