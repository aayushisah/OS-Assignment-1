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
    int waiter_ID;
    printf("Enter Waiter ID: ");
    scanf("%d",&waiter_ID);
}
