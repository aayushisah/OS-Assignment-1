#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include <stdlib.h>
#define BILL "earnings.txt"


int main()
{   
    //key_t key = ftok("/tmp/memfile", 'R');
    //int shmid = shmget(key, sizeof(int), 0666|IPC_CREAT);
    
    

    int ntables;
    printf("Enter the Total Number of Tables at the Hotel: ");
    scanf("%d", &ntables);
    
   int earnings_array[] = {1,2,3};

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
    
    double wages = 0.4*earnings_total;
    double profit = earnings_total - wages;
    
    fprintf(fptr,"Total Earnings of Hotel: %.2f INR", earnings_total);
    fprintf(fptr,"Total Wages of Waiters: %.2f INR", wages);
    fprintf(fptr,"Total Profit: %.2f INR", profit);

    fclose(fptr);
    
    printf("Thank you for visiting the Hotel!");
    exit(0);
}
