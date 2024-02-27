// admin.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_TABLE 10

int main() {
    // Create shared memory segment for termination signal
    key_t adminKey = ftok("admin.c", 'a');
    int adminShmId = shmget(adminKey, sizeof(int), IPC_CREAT | 0666); //can be made read-only
    int *terminate = shmat(adminShmId, NULL, 0);
    if (terminate == (int *)-1) {
        perror("Error in creating/accessing admin shared memory");
        exit(1);
    }

    // Initialize terminate signal
    *terminate = 0;

    char choice;
    do {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No: ");
        scanf(" %c", &choice);
        if (choice != 'Y' && choice != 'N') {
            printf("Invalid choice. Please enter (uppercase) Y or (uppercase) N.\n");
        }
    } while (choice != 'Y');

    if (choice == 'Y') {
        *terminate = 1;
    }

    shmdt(terminate);

    return 0;
}
