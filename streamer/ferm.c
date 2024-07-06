#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {
    sem_t sem;
    sem_init(&sem, 0, 1);
    int value;
    sem_getvalue(&sem, &value);
    printf("%d\n",value);

    return 0;
}
