#include <iostream>
#include <semaphore.h>

using namespace std;

const char* SEM_EMPTY = "/sem_empty";
const char* SEM_FULL = "/sem_full";

void produce_frames() {
    // clear semaphores
    cout << sem_unlink(SEM_EMPTY) << endl;
    cout << sem_unlink(SEM_FULL) << endl;

    // Initialize semaphores
    sem_t *sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, 1);
    sem_t *sem_full = sem_open(SEM_FULL, O_CREAT, 0666, 0);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        std::cerr << "Initialization failed\n";
        return;
    }

    for (int i=0; i<100; i++) {
        cout << "waiting for consumer to read prev frame" << "\n";
        sem_wait(sem_empty);
        cout << "Consumer read prev frame, posting new" << "\n";
        sem_post(sem_full);
    }
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);
}

int main() {
    produce_frames();
    return 0;
}
