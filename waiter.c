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
        perror("Error in fotk\n");
        return 1;
    }
    int shmid = shmget(tablekey, MAX_ORDER, IPC_CREAT | 0666); //do we change MAX_ORDER to sizeof(order)?
    int(*shared_orders)[MAX_ORDER] = shmat(shmid, NULL, 0); //attached to shared memory

    //code to check if order serial numbers exist
    
    //code to check total bill amount and creating new shared memory to send total bill to manager

    //
    }while(ShouldWeContinue != -1)
}
