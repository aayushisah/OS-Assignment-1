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
	printf("printed to file\n");
}

int main() {
 EarningsInfo earnings;
 earnings.table_number = 2;
 earnings.earnings = 50;
 Earnings_to_file(earnings);
}
