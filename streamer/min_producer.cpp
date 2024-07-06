#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

using namespace std;

const char* SHM_NAME = "/vectframe_shm";
const int SHM_SIZE = 10;
const char* SEM_EMPTY = "/sem_empty";
const char* SEM_FULL = "/sem_full";

void produce_frames() {
    // Initialize shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    uint8_t* shm_ptr = (uint8_t*)mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // clear semaphores
    // cout << sem_unlink(SEM_EMPTY) << endl;
    // cout << sem_unlink(SEM_FULL) << endl;

    // Initialize semaphores
    sem_t *sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, 1);
    sem_t *sem_full = sem_open(SEM_FULL, O_CREAT, 0666, 0);

    if (shm_ptr == MAP_FAILED || sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        std::cerr << "Initialization failed\n";
        return;
    }

    // print values in semaphores
    int value;
    sem_getvalue(sem_empty, &value);
    cout << "sem_empty: " << value << "\n";
    sem_getvalue(sem_full, &value);
    cout << "sem_full: " << value << "\n";

    for (int i=0; i<10; i++) {
        cout << "waiting for consumer to read prev frame" << "\n";
        sem_wait(sem_empty);

        for (int j=0; j<10; j++) shm_ptr[j] = i*10 + j;

        cout << "Posting new frame" << "\n";
        sem_post(sem_full);
    }
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);
}

int main() {
    produce_frames();
    return 0;
}
