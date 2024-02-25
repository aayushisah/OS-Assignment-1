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

		// attached to shared memory
		int(*shared_orders)[MAX_ORDER + 1];
		shared_orders = shmat(shmid, NULL, 0);

		int numberOfCustomer = shared_orders[0][1];
		printf("Number of customers = %d\n", numberOfCustomer);

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
						printf("order invalid. \n");
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
					shared_orders[0][0] = 2;
					shared_orders[0][3] = -1; // to make table.c wait for the bill
				}
			}
			while (shared_orders[0][0] == 0)
			{
				printf("Waiting for correction\n");
				sleep(3);
			}
		}
		printf("Order valid!\n");
		shared_orders[0][0] = 2; // returning 2 if order is valid

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

			printf("Bill Amount for Table X: %.2f INR\n", total_bill);
			shared_orders[0][3] = total_bill;

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

			int(*table_bills)[10];
			table_bills = shmat(shmid_bills, NULL, 0);

			// Sending Bill Amount to Manager

			*table_bills[waiterID] = total_bill;
			int amt = *table_bills[waiterID]; // testing
			printf("manager received bill: %d from waiter %d\n", amt, waiterID);

			shmdt(table_bills);
		}

		// Terminate process & detach shared-memory

		shared_orders[0][2] = 0;
		while (shared_orders[0][2] == 0)
		{
		}

		shouldWeContinue = shared_orders[0][2];
		shmdt(shared_orders);

	} while (shouldWeContinue != -1);
}

double* returnPriceArray(){
FILE *file;
char line[100];
itemCount = 0;

file = fopen("menu.txt", "r");
if (file == NULL) {
	printf("Error opening file\n");
	return (double*) 1;
}

//basically this function reads the first one digit
while (fgets(line, sizeof(line), file)) {
	int itemNumber;
	if (sscanf(line, "%d.", &itemNumber) == 1) {
		itemCount++;
	}
}

//why i am using double because i dont know the data type of what can be in menu.txt, can be integer/double so
double* arr = (double*)malloc(itemCount * sizeof(double));
if (arr == NULL) {
	fclose(file);
	return NULL;
}

// Read prices and store in the array
rewind(file); // Reset file pointer to start
 int count=0;
 while (fgets(line, sizeof(line), file)) {
	char priceStr[100];
	double  price;
	//sscanf is returning whatever is after characters, matching with INR, so basically returning string before INR
	if (sscanf(line, "%*d. %*[^0-9]%[^ ] INR",priceStr) == 1) {
		price = atof(priceStr);
		if(count<itemCount)
		arr[count++]=price;
	}
}

fclose(file);
return arr;
}
