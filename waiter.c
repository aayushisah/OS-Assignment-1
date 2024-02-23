#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CUSTOMERS 5 // given in the problem statement maximum number of customers won't exceed 5
#define MAX_TABLE 5		// table size
#define READ_END 0
#define WRITE_END 1
#define MENU "menu.txt"
#define MAX_ORDER 10

int main()
{
	int waiterID;
	printf("Enter Waiter ID: ");
	scanf("%d", &waiterID);
	int shouldWeContinue = 0;

	do
	{
		key_t tablekey;
		int tableID = waiterID;
		if ((tablekey = ftok("table.c", tableID)) == -1)
		{
			perror("Error in ftok\n");
			return 1;
		}
		else
		{
			printf("Generated tablekey..\n");
		}
		int shmid = shmget(tablekey, sizeof(int[MAX_ORDER + 1][MAX_ORDER + 1]), IPC_CREAT | 0666); // do we change MAX_ORDER to sizeof(order)?
		if (shmid == -1)
		{
			perror("Error in creating/accessing shared memory\n");
			return 1;
		}
		else
		{
			printf("shmid also done. It is = %d\n", shmid);
		}

		int(*shared_orders)[MAX_ORDER + 1]; 
		shared_orders = shmat(shmid, NULL, 0); // attached to shared memory
		int numberOfCustomer = shared_orders[0][1];
		printf("Number of customers = %d\n", numberOfCustomer);
		// code to check if order serial numbers exist
		while (shared_orders[0][0] == 0)
		{
			// add while loop here to check for corrected order, add a flag for the above one.
			for (int i = 1; i < numberOfCustomer + 1; i++)
			{
				for (int j = 1; j < MAX_ORDER + 1; j++)
				{
					if (shared_orders[i][j] == -1)
					{
						break;
					}
					if (shared_orders[i][j] < 0 || shared_orders[i][j] > 4)
					{
						printf("order invalid. \n");
						shared_orders[0][0] = -1;
						break;
					}
				}
				if (shared_orders[0][0] == -1)
				{
					break;
				}				
			if(i == numberOfCustomer)
				shared_orders[0][0] = 2;
			}
			while(shared_orders[0][0] == -1){
				printf("Waiting for correction\n");
				sleep(3);
			}
		}
			printf("Order valid!\n");
			shared_orders[0][0] = 2;	//returning 2 if order is valid
		// code to check total bill amount and creating new shared memory to send total bill to manager
		//  wait for valid order if invalid? How?
		if (shared_orders[0][0] == 2)
		{
			int total_bill = 0;
			int prices[4] = {30, 40, 25, 30};

			for (int i = 1; i < numberOfCustomer + 1; i++)
			{
				for (int j = 1; j < MAX_ORDER + 1; j++)
				{
					total_bill += prices[shared_orders[i][j] - 1];
				}
			}

			printf("Bill Amount for Table X: %d INR", total_bill);
			shared_orders[0][3] = total_bill;

			// Creating a shared-memory between Manager-Waiter

		//	key_t billkey;
		//	if ((billkey = ftok("waiter.c", waiterID)) == -1)
		//	{
		//		perror("Error in ftok\n");
			//	return 1;
			//}
			//int shmid_bills = shmget(billkey, sizeof(int) * 10, IPC_CREAT | 0666); // do we change MAX_ORDER to sizeof(order)?
			//if (shmid_bills == -1)
			//{
				//perror("Error in creating/accessing shared memory\n");
			//	return 1;
			//}

			//int table_bills[10];
//			table_bills = shmat(shmid_bills, NULL, 0);
//
//			// Sending Bill Amount to Manager
//
//			table_bills[waiterID] = total_bill;
//			shmdt(table_bills);
		}

		// Termination
		shared_orders[0][2] = 0;
		while(shared_orders[0][2] == 0)
		{
		}
		
		shouldWeContinue = shared_orders[0][2];
		shmdt(shared_orders);
	} while (shouldWeContinue != -1);
}
