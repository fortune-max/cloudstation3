#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

using namespace std;

const char* SHM_NAME = "/vectframe_shm";
const int SHM_SIZE = 10;
const char* SEM_EMPTY = "/sem_empty";
const char* SEM_FULL = "/sem_full";

void consume_frames() {
    // Open shared memory
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    uint8_t* shm_ptr = (uint8_t*)mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    // Open semaphores
    sem_t *sem_empty = sem_open(SEM_EMPTY, 0);
    sem_t *sem_full = sem_open(SEM_FULL, 0);

    if (shm_ptr == MAP_FAILED || sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        std::cerr << "Initialization failed\n";
        return;
    }

    for (int i=0; i<10; i++) {
        cout << "Waiting for frame" << "\n";
        sem_wait(sem_full);

        cout << "Frame received: ";
        for (int j=0; j<10; j++) cout << (int)shm_ptr[j] << " ";
        cout << "\n";

        cout << "Frame received" << "\n";
        sem_post(sem_empty);
    }
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_empty);
    sem_close(sem_full);
}

int main() {
    consume_frames();
    return 0;
}
