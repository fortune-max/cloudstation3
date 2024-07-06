#include <iostream>
#include <semaphore.h>

using namespace std;

const char* SEM_EMPTY = "/sem_empty";
const char* SEM_FULL = "/sem_full";

void consume_frames() {
    // Open semaphores
    sem_t *sem_empty = sem_open(SEM_EMPTY, 0);
    sem_t *sem_full = sem_open(SEM_FULL, 0);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        std::cerr << "Initialization failed\n";
        return;
    }

    for (int i=0; i<100; i++) {
        cout << "Waiting for frame" << "\n";
        sem_wait(sem_full);
        cout << "Frame received" << "\n";
        sem_post(sem_empty);
    }
    sem_close(sem_empty);
    sem_close(sem_full);
}

int main() {
    consume_frames();
    return 0;
}
