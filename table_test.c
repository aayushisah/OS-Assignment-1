#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define MAX_CUSTOMERS 5
#define MAX_ITEMS 4
#define MAX_ITEM_NAME_LEN 50
#define MAX_ORDER_LEN 100


// Structure to hold menu items
typedef struct {
    char name[MAX_ITEM_NAME_LEN];
    int price;
} MenuItem;

// Function to read menu from file
void read_menu(MenuItem menu[]) {
    FILE *file = fopen("menu.txt", "r");
    if (file == NULL) {
        perror("Error opening menu file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (fscanf(file, "%s %d INR\n", menu[i].name, &menu[i].price) != 2) {
            perror("Error reading menu file");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
}

// Function to display menu
void display_menu(MenuItem menu[]) {
    printf("Menu:\n");
    for (int i = 0; i < MAX_ITEMS; ++i) {
        printf("%d. %s %d INR\n", i + 1, menu[i].name, menu[i].price);
    }
}

// Function to create shared memory
int create_shared_memory() {
    int fd = shm_open("/table_shm", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, sizeof(int)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int main() {
    MenuItem menu[MAX_ITEMS];
    read_menu(menu);

    while (1) {
        // Step b: Get table number from user
        int table_number;
        printf("Enter Table Number: ");
        scanf("%d", &table_number);

        // Step c: Get number of customers
        int num_customers;
        printf("Enter Number of Customers at Table (maximum no. of customers can be 5): ");
        scanf("%d", &num_customers);

        // Step c: Create pipes for communication with customers
        int pipes[MAX_CUSTOMERS][2];
        for (int i = 0; i < num_customers; ++i) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        // Step d: Display menu
        display_menu(menu);

        // Step e: Create customer processes and handle orders
        for (int i = 0; i < num_customers; ++i) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) { // Child process (customer)
                close(pipes[i][0]); // Close read end of the pipe
                printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");
                int item_number;
                char order[MAX_ORDER_LEN] = "";
                do {
                    scanf("%d", &item_number);
                    if (item_number >= 1 && item_number <= MAX_ITEMS) {
                        strcat(order, menu[item_number - 1].name);
                        strcat(order, ", ");
                    }
                } while (item_number != -1);
                write(pipes[i][1], order, strlen(order) + 1);
                close(pipes[i][1]); // Close write end of the pipe
                exit(EXIT_SUCCESS);
            } else { // Parent process (table)
                close(pipes[i][1]); // Close write end of the pipe
            }
        }

        // Step f: Create and initialize shared memory for bill communication with waiter
        int shm_fd = create_shared_memory();
        int *shared_bill = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shared_bill == MAP_FAILED) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        // Step g: Wait for all customers to finish and gather orders
        char orders[MAX_CUSTOMERS][MAX_ORDER_LEN];
        for (int i = 0; i < num_customers; ++i) {
            read(pipes[i][0], orders[i], sizeof(orders[i]));
            wait(NULL); // Wait for child process to finish
        }

        // Step f: Communicate order to waiter via shared memory
        printf("Order communicated to waiter for table %d\n", table_number);
        // In this simplified example, let's assume the bill is the sum of all item prices
        int total_bill = 0;
        for (int i = 0; i < num_customers; ++i) {
            char *token = strtok(orders[i], ",");
            while (token != NULL) {
                for (int j = 0; j < MAX_ITEMS; ++j) {
                    if (strcmp(token, menu[j].name) == 0) {
                        total_bill += menu[j].price;
                        break;
                    }
                }
                token = strtok(NULL, ",");
            }
        }
        *shared_bill = total_bill;

        // Step h: Display total bill amount
        printf("The total bill amount is %d INR.\n", *shared_bill);

        // Clean up shared memory
        munmap(shared_bill, sizeof(int));
        close(shm_fd);
        shm_unlink("/table_shm");

        // Step i: Ask user if more customers will sit at this table
        printf("Do you wish to seat a new set of customers at the table? Enter -1 to exit or a number between 1 and 5 for new customers: ");
        scanf("%d", &num_customers);
        if (num_customers == -1) {
            // Terminate waiter process
            printf("Not seating new customers. Informing waiter to terminate.\n");
            break;
        }
        // Cleanup pipes
        for (int i = 0; i < num_customers; ++i) {
            close(pipes[i][0]); // Close read end of the pipe
        }
    }

    return 0;
}
