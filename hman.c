#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <strings.h>

#define MAX_TABLES 10 // Maximum number of tables
#define SHM_KEY 12345 //temp key for admin-manager shm

// Structure for holding earnings information
typedef struct {
    int table_number;
    int earnings;
} EarningsInfo;

// Function to write earnings to file and print them on console
void write_earnings_to_file(EarningsInfo earnings[], int num_tables, int total_earnings) {
    FILE *file = fopen("earnings.txt", "w");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_tables; i++) {
        fprintf(file, "Earning from Table %d: %d INR\n", earnings[i].table_number, earnings[i].earnings);
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

    // Print total earnings, total wages, and total profit on console
    printf("Total Earnings of Hotel: %d INR\n", total_earnings);
    printf("Total Wages of Waiters: %d INR\n", total_wages);
    printf("Total Profit: %d INR\n", total_profit);

    fclose(file);
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
    EarningsInfo earnings[MAX_TABLES];
    int total_earnings = 0;

    // Initialize earnings for each table to 0
    for (int i = 0; i < num_tables; i++) {
        earnings[i].table_number = i + 1; // Assuming table numbers start from 1
        earnings[i].earnings = 0;
    }
    // Create shared memory segment to receive earnings from waiters
    int waiterID;
    
    for(int i=0; i<num_tables; i++ ){
    waiterID= earnings[i].table_number ;
        
    key_t billkey;
    if ((billkey = ftok("waiter.c", waiterID)) == -1) {
        perror("Error in ftok\n");
        return 1;
    }

    int shmid_bills = shmget(billkey, sizeof(int) * 10, IPC_CREAT | 0666);
    if (shmid_bills == -1) {
        perror("Error in creating/accessing shared memory\n");
        return 1;
    }

    // Attach shared memory segment
    int(*table_bills)[10];
    table_bills = shmat(shmid_bills, NULL, 0);

    // Read earnings from waiters and update the earnings for each table
        earnings[i].earnings = (*table_bills)[i];
        total_earnings += earnings[i].earnings;

    // Detach shared memory segment
    shmdt(table_bills);
    }

    //Creating shared memory between admin and manager
    int *terminateHotel;
    key_t terminationkey;
    if ((terminationkey = ftok("admin.c", SHM_KEY)) == -1) {
        perror("Error in ftok\n");
        return 1;
    }
    int shmid = shmget(terminationkey, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error in creating/accessing shared memory\n");
        exit(EXIT_FAILURE);
    }
    
    terminateHotel = (int *)shmat(shm_id, NULL, 0);

    // Write earnings to file and print them on console
    write_earnings_to_file(earnings, num_tables, total_earnings);

    // Display terminating message
    printf("Thank you for visiting the Hotel!\n");

    return 0;
}
