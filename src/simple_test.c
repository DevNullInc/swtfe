#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#define TEST_MACRO 1
#else /* __cplusplus */  
#define TEST_MACRO 0
#endif

int main() {
    printf("Test successful\n");
    return 0;
}
