#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include<string.h>

#define MAX_TABLES 10 // Maximum number of tables
#define SHM_KEY 12345 //temp key for shared memory btwn admin and manager

// Structure for holding earnings information
typedef struct {
    int table_number;
    int earnings;
} EarningsInfo;

// Function to write earnings to file and print them on console
void Earnings_to_file(EarningsInfo e) {
    FILE *file = fopen("earnings.txt", "w");
    if (file == NULL) {
        printf("fopen");
        exit(EXIT_FAILURE);
    }

    
        fprintf(file, "Earning from Table %d: %d INR\n", e.table_number, e.earnings);
    

    
}

int main() {
    int num_tables;

    // Prompt user to enter the number of tables
    printf("Enter the total number of tables at the hotel (max %d): ", MAX_TABLES);
    scanf("%d", &num_tables);
    printf("conkey");
    if (num_tables <= 0 || num_tables > MAX_TABLES) {
        printf("Invalid number of tables. Exiting...\n");
        return 1;
    }
    printf("donkey");
    // Placeholder for actual earnings calculation
    EarningsInfo earnings[num_tables];
    int total_earnings = 0;

    // Initialize earnings for each table to 0
    for (int i = 0; i < num_tables; i++) {
        earnings[i].table_number = i + 1; // Assuming table numbers start from 1
        earnings[i].earnings = 0;
    }
    printf("monkey");
 //shared memory between admin and hotel manager
    int *terminateHotel;
    key_t terminationkey;
    if ((terminationkey = ftok("admin.c", SHM_KEY)) == -1) {
        printf("Error in ftok\n");
        return 1;
    }
    
    int shmid = shmget(terminationkey, sizeof(int), IPC_CREAT | 0666);
    printf("shmid is %d", shmid);
    if (shmid == -1) {
        printf("Error in creating/accessing shared memory\n");
        exit(EXIT_FAILURE);
    }
    printf("hi guys");
    
    terminateHotel = (int *)shmat(shmid, NULL, 0);
    // Create shared memory segment to receive earnings from waiters
     int count=0;
    int waiterID;
    while( terminateHotel!=0 || count!=num_tables)
        {for(int i=0; i<num_tables; i++ ){
            waiterID= i+1;
            key_t billkey;
            if ((billkey = ftok("waiter.c", waiterID)) == -1) {
                printf("Error in ftok\n");
                return 1;
            }

            int shmid_bills = shmget(billkey, sizeof(int), IPC_CREAT | 0666);
            if (shmid_bills == -1) {
                printf("Error in creating/accessing shared memory\n");
                return 1;
            }   
            printf("hman is connecged to %d\n", shmid_bills);
            //masterplan
            //run loop for tables 1-X
            //create a mem segment of waiterID (unique)
            //it has only onw bit, which is 0 (no cust at table) or bill amount (non-zero)
            //if bill amount; send to earnings.txt, set it to zero, continue to next table
            //if its zero, continue to next table
            

            // Attach shared memory segment
            int *table_bills;
            table_bills = shmat(shmid_bills, NULL, 0);

           // printf("table ka bill is %d", table_bills);
            if(*table_bills==0)  //-1 means no customer at that table rn
            {
                count++;
                shmdt(table_bills);
                printf("it thinks im 0");

                 continue;
             }
            // Read earnings from waiters and update the earnings for each table
            if(*table_bills!=0){
                earnings[i].earnings = *table_bills;
                total_earnings += earnings[i].earnings;
                Earnings_to_file(earnings[i]);
                printf("bill received from table %d = %d", i+1,*table_bills);
                *table_bills = 0;
            }
            // Detach shared memory segment
            shmdt(table_bills);
        }
    }
   
    FILE *file = fopen("earnings.txt", "w");
    if (file == NULL) {
        printf("fopen");
        exit(EXIT_FAILURE);
    }
    // Write total earnings to file
    fprintf(file, "Total Earnings of Hotel: %d INR\n", total_earnings);

    // Assuming total wages is 40% of total earnings
    int total_wages = total_earnings * 0.4;

    // Write total wages to file
    fprintf(file, "Total Wages of Waiters: %d INR\n", total_wages);

    // Calculate and write total profit to file
    int total_profit = total_earnings - total_wages;
    fprintf(file, "Total Profit: %d INR\n", total_profit);


    fclose(file);



    // Display terminating message
    printf("Thank you for visiting the Hotel!\n");

    return 0;
}
