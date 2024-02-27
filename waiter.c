#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CUSTOMERS 5 // given in the problem statement maximum number of customers won't exceed 5
#define MAX_TABLE 10		// table size
#define READ_END 0
#define WRITE_END 1
#define MENU "menu.txt"
#define MAX_ORDER 10

double* returnPriceArray();
int itemCount;

int main()
{
	int waiterID;
	printf("Enter Waiter ID: ");
	scanf("%d", &waiterID);
	int shouldWeContinue = 0;

	double* prices = returnPriceArray();

	do
	{
		key_t tablekey;
		int tableID = waiterID;
		if ((tablekey = ftok("table.c", tableID)) == -1)
		{
			perror("Error in ftok\n");
			return 1;
		}
		
		int shmid = shmget(tablekey, sizeof(int[MAX_ORDER + 1][MAX_ORDER + 1]), IPC_CREAT | 0666); 
		if (shmid == -1)
		{
			perror("Error in creating/accessing shared memory\n");
			return 1;
		}


		// attached to shared memory
		int(*shared_orders)[MAX_ORDER + 1];
		shared_orders = shmat(shmid, NULL, 0);

		int numberOfCustomer = shared_orders[0][1];
		
        	while(numberOfCustomer==0){
            		numberOfCustomer = shared_orders[0][1];
        	}

        // shared_orders flag assigment :
        // [0][0] checks order validity.
        // [0][1] sends the number of customers.
        // [0][2] shouldWeContinue (are more customers coming or is the table closed for the day.
        // [0][3] total bill amount.
        // [0][4] holds the waiter until an order is ready to be calculated.
        // [0][5] informs the waiter that the table has terminated for the day.

		// code to check if order serial numbers exist
		while (shared_orders[0][0] == -1)
		{
			for (int i = 1; i < numberOfCustomer + 1; i++)
			{
				for (int j = 1; j < MAX_ORDER + 1; j++)
				{
					if (shared_orders[i][j] == -1)
					{
						break;
					}
					if (shared_orders[i][j] < 1 || shared_orders[i][j] > itemCount)
					{
						//printf("order invalid. \n");
						shared_orders[0][0] = 0;
						break;
					}
				}
				if (shared_orders[0][0] == 0)
				{
					break;
				}
				if (i == numberOfCustomer)
				{
					shared_orders[0][0] = 2;  // order is valid
					shared_orders[0][3] = -1; // to make table.c wait for the bill
				}
			}
			while (shared_orders[0][0] == 0)
			{
				//sleep(3);
			}
		}
		//printf("Order valid!\n");
		shared_orders[0][0] = 2; // returning 2 if order is valid

        int (*table_bills);
		// check total bill and creating new shared memory to send total bill to manager
		if (shared_orders[0][0] == 2)
		{
			double total_bill = 0;

			for (int i = 1; i < numberOfCustomer + 1; i++)
			{
				for (int j = 1; j < MAX_ORDER + 1; j++)
				{
					if (shared_orders[i][j] == -1)
						break;
					total_bill += prices[shared_orders[i][j] - 1];
				}
			}

			printf("Bill Amount for Table %d is: %.2f INR\n", waiterID, total_bill);
			shared_orders[0][3] = total_bill;

			// Creating a shared-memory between Hotel Manager & Waiter

			key_t billkey;
			if ((billkey = ftok("waiter.c", waiterID)) == -1)
			{
				perror("Error in ftok\n");
				return 1;
			}

			int shmid_bills = shmget(billkey, sizeof(int), IPC_CREAT | 0666);
			if (shmid_bills == -1)
			{
				perror("Error in creating/accessing shared memory\n");
				return 1;
			}
		
			table_bills = shmat(shmid_bills, NULL, 0);

			// Sending Bill Amount to Manager
			if(numberOfCustomer != 0 && (int)total_bill==0){
				*table_bills = -1;
			}
			else{
				*table_bills = (int)total_bill;
			}

		}

		shared_orders[0][2] = 0;
		while (shared_orders[0][2] == 0)
		{
            //sleep(2);
		}

		shouldWeContinue = shared_orders[0][2];
        while (shared_orders[0][4] != 1){ // while its not ready to take order, also here is where it gets flag to terminate
            if(shared_orders[0][5]==1){
                *table_bills = -2;
                shouldWeContinue = -1;
                shared_orders[0][1] = 0;
                break;
            }
            //sleep(2);
        }
        
        
		shmdt(shared_orders);
        shmdt(table_bills);

	} while(shouldWeContinue != -1);

	return 0;
}

double* returnPriceArray()
{
	FILE *file;
	char line[100];
	itemCount = 0;

	file = fopen("menu.txt", "r");
	if (file == NULL)
	{
		printf("Error opening file\n");
		return (double*)1;
	}

	//basically this function reads the first one digit
	while (fgets(line, sizeof(line), file))
	{
		int itemNumber;
		if (sscanf(line, "%d.", &itemNumber) == 1)
		{
			itemCount++;
		}
	}

	//why i am using double because i dont know the data type of what can be in menu.txt, can be integer/double so
	double* arr = (double*)malloc(itemCount * sizeof(double));
	if (arr == NULL)
	{
		fclose(file);
		return NULL;
	}

	// Read prices and store in the array
	rewind(file); // Reset file pointer to start
	int count = 0;
	while (fgets(line, sizeof(line), file))
	{
		char priceStr[100];
		double  price;
		//sscanf is returning whatever is after characters, matching with INR, so basically returning string before INR
		if (sscanf(line, "%*d. %*[^0-9]%[^ ] INR", priceStr) == 1)
		{
			price = atof(priceStr);
			if (count < itemCount)
				arr[count++] = price;
		}
	}

	fclose(file);
	return arr;
}
