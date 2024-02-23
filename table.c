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

void menuRead(); // func_declaration for function to read the menu
int **ordersArr(int numberOfCustomer);
int main()
{
    int tableNumber;
    int numberOfCustomer;
    // tableNumber being assigned by the user itself is guaranteed
    // Assuming user always gives a valid input
    printf("Enter Table Number: ");
    scanf("%d", &tableNumber); // between 1 and 10 inclusive
    int ShouldWeContinue = 0;
    // the table process requests to know the number of customers
    do
    {
        printf("\nEnter Number of Customers at Table (maximum no. of customers can be 5): ");
        scanf("%d", &numberOfCustomer); // between 1 and 5 inclusive

        // displaying the menu to the customer
        menuRead();

        // Dynamically allocate memory for orders array
        int **orders = ordersArr(numberOfCustomer);

        // shared memory segment
        int tableId = tableNumber;
        key_t tablekey; // key to identify shared memory segment
        // Generate a key for the shared memory segment
        if ((tablekey = ftok("table.c", tableId)) == -1)
        {
            perror("Error in ftok\n");
            return 1;
        }

        int shmid = shmget(tablekey, sizeof(int) * (MAX_ORDER + 1), IPC_CREAT | 0666); // read and write both permission given
        if (shmid == -1)
        {
            perror("Error in shmget in creating/ accessing shared memory\n");
            return 1;
        }

        int(*shared_orders)[MAX_ORDER + 1] = shmat(shmid, NULL, 0); // 2d array to store orders and this is basically passed to the shared segment between waiter and table
        if (shared_orders == (void *)-1)
        {
            perror("Error in shmPtr in attaching the memory segment\n");
            return 1;
        }
        // Copy orders data to shared memory
        // shared_orders[0][0] to be empty , it will either show valid order or invalid order, in case of valid order will store the bill
        shared_orders[0][0] = 0;
        shared_orders[0][1] = numberOfCustomer; // aditya added, delete if causing trouble.
        for (int i = 1; i < numberOfCustomer + 1; i++)
        {
            for (int j = 1; j < MAX_ORDER + 1; j++)
            {
                shared_orders[i][j] = orders[i][j];
            }
        }

        // Wait for waiter to check order validity
        while (shared_orders[0][0] == 0)
        {
            sleep(1);
        }
        // shared_orders[0][0]=-1 means invalid
        while (shared_orders[0][0] == -1)
        {
            // Order is invalid, prompt customers to give orders again
            printf("Invalid order detected. Please give orders again.\n");
            orders = ordersArr(numberOfCustomer);
            // Copy orders data to shared memory
            for (int i = 1; i < numberOfCustomer + 1; i++)
            {
                for (int j = 1; j < MAX_ORDER + 1; j++)
                {
                    shared_orders[i][j] = orders[i][j];
                }
            }
        }
        // asking the table do we want more customers, end it if we get -1
        printf("Do you want more customers?");
        scanf("%d", &ShouldWeContinue);
    } while (ShouldWeContinue != -1);
}

// pipe creation
int **ordersArr(int numberOfCustomer)
{
    int **orders = (int **)malloc(sizeof(int *) * numberOfCustomer);
    for (int i = 0; i < numberOfCustomer; i++)
    {
        orders[i] = (int *)malloc(sizeof(int) * MAX_ORDER);
    }
    // creation of child process
    pid_t pid;
    // creation of pipes for communication b/w a table process and each customer
    // so there's a read end and write end in a pipe and we need a pipe for each customer
    int fd[numberOfCustomer][2]; // int fd[MAX_CUSTOMERS][2]
    for (int i = 0; i < numberOfCustomer; i++)
    {
        // if pipe creation failed terminate
        if (pipe(fd[i]) == -1)
        {
            fprintf(stderr, "Pipe creation failed\n");
            return 1;
        }
    }
    for (int i = 0; i < numberOfCustomer; i++)
    {

        // fork a child process
        pid = fork();

        // error occured
        if (pid < 0)
        {
            fprintf(stderr, "Fork Failed");
            return 1;
        }
        else if (pid == 0)
        {                           // child process
            close(fd[i][READ_END]); // customer is writing on the pipe, close the unused end
            int food_itm = 0;
            while (food_itm != -1)
            {
                printf("%d", i); /// comment it outt for correct output format, debugging purpose
                printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");
                scanf("%d", &food_itm);
                write(fd[i][WRITE_END], &food_itm, sizeof(food_itm));
            }
            close(fd[i][WRITE_END]); // close write end before exiting
            return 0;
        }
        wait(NULL);
    }
    // Table process reading the pipe
    for (int i = 0; i < numberOfCustomer; i++)
    {
        close(fd[i][WRITE_END]);
        int food_itm;
        int order_count = 0;
        // read till you encounter -1
        while (read(fd[i][READ_END], &food_itm, sizeof(food_itm)) > 0)
        {
            orders[i][order_count++] = food_itm;
        }
        close(fd[i][READ_END]);
    }
    return orders;
}

// Menu read function
void menuRead()
{
    FILE *file;
    char lines[100];

    // reading the menu
    file = fopen(MENU, "r");
    if (file == NULL)
    {
        printf("Error opening menu.txt\n");
    }

    // On successful opening of menu.txt
    while (fgets(lines, sizeof(lines), file))
    {
        printf("%s", lines);
    }
    printf("\n");
    // close the file
    fclose(file);
}
