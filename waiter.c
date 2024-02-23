#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CUSTOMERS 5 // given in the problem statement maximum number of customers won't exceed 5
#define MAX_TABLE 5     // table size
#define READ_END 0
#define WRITE_END 1
#define MENU "menu.txt"
#define MAX_ORDER 50

int main(){
    int waiterID;
    printf("Enter Waiter ID: ");
    scanf("%d",&waiterID);
    int ShouldWeContinue = 0;
    
    do{
    key_t tablekey;
    if((tablekey = ftok("table.c", waiterID)) == -1){
        perror("Error in ftok\n");
        return 1;
    }
    int shmid = shmget(tablekey, MAX_ORDER, IPC_CREAT | 0666); //do we change MAX_ORDER to sizeof(order)?
	if(shmid==-1){
		perror("Error in creating/accessing shared memory\n");
		return 1;
	} 	

    int(*shared_orders)[MAX_ORDER+1] = shmat(shmid, NULL, 0); //attached to shared memory
	int numberOfCustomer = shared_orders[0][1];
    //code to check if order serial numbers exist
	for(int i=1;  i < numberOfCustomer+1; i++)
	{
	    for(int j = 1; j < MAX_ORDER + 1; j++)
	    {
	    	if(shared_orders[i][j] == -1){
		    break;
		}
			if(shared_orders[i][j] < 1 || shared_orders[i][j] > 4)
			{
			    shared_orders[i][j] = -1;
			}
	    }  
	}
    //code to check total bill amount and creating new shared memory to send total bill to manager
	// wait for valid order if invalid? How?
	int total_bill = 0;
	int prices[4] = {30, 40, 25, 30};
	
	for(int i = 1; i < numberOfCustomer + 1; i++)
	{
		for(int j = 1; j < MAX_ORDER + 1; j++)
		{
			total_bill += prices[shared_orders[i][j]-1]; 
		}	
	}
	
	printf("Bill Amount for Table X: %d INR", total_bill);

    }while(ShouldWeContinue != -1)
}
