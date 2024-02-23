#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#define BILL "earnings.txt"


int main()
{   
  // Creating a shared-memory between Manager-Waiter

			key_t billkey;
			if ((billkey = ftok("waiter.c", waiterID)) == -1)
			{
				perror("Error in ftok\n");
				return 1;
			}
			int shmid_bills = shmget(billkey, sizeof(int) * 10, IPC_CREAT | 0666);
			if (shmid_bills == -1)
			{
				perror("Error in creating/accessing shared memory\n");
				return 1;
			}

	         int(*table_bills)[10] = shmat(shmid_bills, NULL, 0);

             shmdt(table_bills);

	// Sending Bill Amount to Manager

	//table_bills[WAITER_ID] = total_bill;

    int ntables;
    printf("Enter the Total Number of Tables at the Hotel: ");
    scanf("%d", &ntables);
    
   int earnings_array[ntables-1];

    FILE *fptr;
    fptr = fopen("earnings.txt", "w");

    for (int i = 0; i < ntables; i++) {
        fprintf(fptr, "Earning from Table %d: %d INR\n", earnings_array[i]);
    }

    double earnings_total = 0;
    
    for (int i=0; i < ntables; i++)
    {
        earnings_total += earnings_array[i];
    }
    
    double total_wages = 0.4*earnings_total;
    double total_profit = earnings_total - wages;
    
    fprintf(fptr,"Total Earnings of Hotel: %.2f INR\n", earnings_total);
    fprintf(fptr,"Total Wages of Waiters: %.2f INR\n", total_wages);
    fprintf(fptr,"Total Profit: %.2f INR\n", total_profit);

    fclose(fptr);
    
    printf("Thank you for visiting the Hotel!");
    exit(0);
}

