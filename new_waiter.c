#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CUSTOMERS 5 // given in the problem statement maximum number of customers won't exceed 5
#define MAX_TABLE 5     // table size
#define READ_END 0
#define WRITE_END 1
#define MENU "menu.txt"
#define MAX_ORDER 10
int CheckValidity(int (*shared_orders)[MAX_ORDER + 1]);

int main() {
    int waiterId;
    printf("Enter Waiter ID: ");
    scanf("%d", &waiterId);

    int tableId = waiterId; // Assuming waiterId corresponds to the tableId
    key_t tablekey;
    if ((tablekey = ftok("table.c", tableId)) == -1) {
        perror("Error in ftok\n");
        return 1;
    }

    int shmid = shmget(tablekey, sizeof(int[MAX_ORDER+1][MAX_ORDER+1]), 0666);
    if (shmid == -1) {
        perror("Error in shmget in creating/ accessing shared memory\n");
        return 1;
    }

    int (*shared_orders)[MAX_ORDER + 1];
    shared_orders = shmat(shmid, 0, 0);
    if (shared_orders == (void *)-1) {
        perror("Error in shmPtr in attaching the memory segment\n");
        return 1;
    }

    int NumberOfCustomers = shared_orders[0][1];
    do {
    // Check the validity of the order
    shared_orders[0][0] = CheckValidity(shared_orders);

    // Print a message for debugging
    printf("Order validity: %d\n", shared_orders[0][0]);

    // If the order is invalid, set a specific value to indicate the need to retake orders
    if (shared_orders[0][0] == 0) {
        // Print a message for debugging
        printf("Invalid order detected. Waiting...\n");

        // Add a sleep to avoid busy-waiting
        sleep(5); // Adjust the sleep duration as needed
    }

    // Add any necessary synchronization or sleep statements here if needed

} while (shared_orders[0][0] !=1); // Adjust the loop condition based on your specific requirements


    // Detach shared memory
    shmdt(shared_orders);
    // destroy the shared memory
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

int CheckValidity(int (*shared_orders)[MAX_ORDER + 1]) {
    // Assuming menu is an array containing valid serial numbers
    int isValidOrder = 1;
    for (int i = 1; i < MAX_TABLE+1; i++) {
        for (int j = 1; j < MAX_ORDER+1; j++) {
            int orderItem = shared_orders[i][j];
            if (orderItem == -1)
            {
            	break;
            }  // Skip empty slots in the order

            // Check if the order item is valid (exists in the menu)
            else if (orderItem < 1 || orderItem > 4) {
                isValidOrder = 0;
                break;
            }
        }
    }

    return isValidOrder;
}
