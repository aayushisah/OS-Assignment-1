#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define SHM_KEY 12345 // temporary key for shared memory segment

int main() {
    char choice;
    int *terminateHotel;
    int shm_id;

    ftok("admin.c", SHM_KEY);
    // Create shared memory segment
    shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory segment
    terminateHotel = (int *)shmat(shm_id, NULL, 0);
    if (terminateHotel == (int *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No: ");
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y') {
            *terminateHotel = 1; // Inform hotel manager to terminate
            printf("Hotel closure request sent to Hotel Manager.\n");
            break; // Exit admin process
        } else if (choice == 'N' || choice == 'n') {
            // Continue running admin process
            continue;
        } else {
            printf("Invalid input. Please enter Y or N.\n");
        }
    }

    // Detach and remove shared memory segment

    /*
    * clean up process for shared memory has to be completed
    */
    if (shmdt(terminateHotel) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}
