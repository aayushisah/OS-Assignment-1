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

void Earnings_to_file(EarningsInfo e) {
    FILE *file = fopen("earnings.txt", "a");
    
    if (file == NULL) {
        perror("Error opening file");
        return;  // Return gracefully instead of using exit
    }

    // Write to file
    fprintf(file, "Earning from Table %d: %d INR\n", e.table_number, e.earnings);
    
    // Close the file
    fclose(file);

    // Print to console
    printf("Earning from Table %d: %d INR\n", e.table_number, e.earnings);
}

int main() {
    int num_tables;

    // Prompt user to enter the number of tables
    printf("Enter the total number of tables at the hotel (max %d): ", MAX_TABLES);
    scanf("%d", &num_tables);
    if (num_tables <= 0 || num_tables > MAX_TABLES) {
        printf("Invalid number of tables. Exiting...\n");
        return 1;
    }
    // Placeholder for actual earnings calculation
    EarningsInfo bills;
    int total_earnings = 0;

    //shared memory between admin and hotel manager
    key_t adminKey = ftok("admin.c", 'a');
    int adminShmId = shmget(adminKey, sizeof(int), IPC_CREAT | 0666);
    int *terminate = shmat(adminShmId, NULL, 0);
    if (terminate == (int *)-1) {
        perror("Error in creating/accessing admin shared memory");
        exit(1);
    }

   
    // Create shared memory segment to receive earnings from waiters
    int count=num_tables; //active tables
    int waiterID;
    while(count>0 || *terminate==0)
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
            

            // Attach shared memory segment
            int *table_bills;
            table_bills = shmat(shmid_bills, NULL, 0);
            if(*table_bills==0) 
            {
				shmdt(table_bills);
                continue;
             }
			else if(*table_bills==-1){
				bills.table_number = i+1;
                bills.earnings = 0;        
			  	total_earnings += bills.earnings;
                *table_bills = 0;
                Earnings_to_file(bills);
			}
            else if(*table_bills==-2){
                count--;
                //printf("table %d closed, %d remain active\n", i+1, count);
                *table_bills = 0;
            }
            // Read earnings from waiters and update the earnings for each table
			else{
				bills.table_number = i+1;
                bills.earnings = *table_bills;
                total_earnings += bills.earnings;
                *table_bills = 0;
                Earnings_to_file(bills);
            }
            // Detach shared memory segment
			shmdt(table_bills);
        }
    }
    shmdt(terminate);
   
    FILE *file = fopen("earnings.txt", "a");
    if (file == NULL) {
        printf("fopen");
        exit(EXIT_FAILURE);
    }
    // Write total earnings to file
    fprintf(file, "\nTotal Earnings of Hotel: %d INR\n", total_earnings);
    printf("\nTotal Earnings of Hotel: %d INR\n", total_earnings);

    // Assuming total wages is 40% of total earnings
    int total_wages = total_earnings * 0.4;

    // Write total wages to file
    fprintf(file, "Total Wages of Waiters: %d INR\n", total_wages);
    printf("Total Wages of Waiters: %d INR\n", total_wages);

    // Calculate and write total profit to file
    int total_profit = total_earnings - total_wages;
    fprintf(file, "Total Profit: %d INR\n", total_profit);
    printf("Total Profit: %d INR\n", total_profit);

    fclose(file);

    // Display terminating message
    printf("Thank you for visiting the Hotel!\n");

    return 0;
}
