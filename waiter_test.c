#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_ORDER 10
#define MAX_TABLE 5

int main() {
    int waiterId;
    printf("Enter Waiter ID: ");
    scanf("%d", &waiterId);

    int tableId = waiterId; // Assuming waiterId corresponds to the tableId
    key_t tablekey = ftok("table.c", tableId);
    	if ((tablekey = ftok("table.c", tableId)) == -1)
        {
            perror("Error in ftok\n");
            return 1;
        }
    	int shmid = shmget(tablekey, sizeof(int[MAX_ORDER+1][MAX_ORDER+1]), 0666); 
    	if (shmid == -1)
        {
            perror("Error in shmget in creating/ accessing shared memory\n");
            return 1;
        }
  	int (*shared_orders)[MAX_ORDER + 1];
        shared_orders = shmat(shmid, 0, 0);
    	if (shared_orders == (void *)-1)
       {
            perror("Error in shmPtr in attaching the memory segment\n");
            return 1;
       }
    
        int NumberOfCustomers=shared_orders[0][1];
        // Check if the order is valid
        //position 0 1 is showing there are customers in table process still
        while(shared_orders[0][1]==1){
        	int isValidOrder = -1;
        	while(shared_orders[0][0]==0){
        		for (int j = 1; j < NumberOfCustomers+1; j++) {
            			for (int i = 1; i < MAX_ORDER+1; i++) {
            			printf("%d " ,shared_orders[j][i]); 
                		if (shared_orders[j][i] < 1 || shared_orders[j][i] > 4) {
                    			isValidOrder = 0;
                    			break;
                }
                printf("\n");
            }

            	if (isValidOrder==0) {
                // Communicate to the table process that the order is invalid
                printf("returning invalid order to the table process");
                shared_orders[0][0] = -1;
            	}
            	else{ 
            	printf("returning valid order to the table process");
            	isValidOrder=0;
            	shared_orders[0][0]=2;}
            }
            }
        }
    

    // Detach shared memory
    shmdt(shared_orders);
	// destroy the shared memory
    shmctl(shmid, IPC_RMID, NULL);
}