#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#define MAX_CUSTOMERS 5 // given in the problem statement maximum number of customers won't exceed 5
#define MAX_TABLE 10    // table size can be a maximum of 10; monotonically increasing
#define READ_END 0      // for the file descriptor
#define WRITE_END 1
#define MENU "menu.txt"
#define MAX_ORDER 10

void menuRead(); // func_declaration for function to read the menu
int **ordersArr(int numberOfCustomer);
int main()
{
    int tableNumber;
    int numberOfCustomer;
    // tableNumber being assigned by the user itself is guaranteed
    // Assuming user always gives a valid input
    printf("Enter Table Number:");
    scanf("%d", &tableNumber); // between 1 and 10 inclusive
    int ShouldWeContinue = 0;
    // the table process requests to know the number of customers
    // shared memory segment
    int tableId = tableNumber;
    key_t tablekey; // key to identify shared memory segment
                    // Generate a key for the shared memory segment
    if ((tablekey = ftok("table.c", tableId)) == -1)
    {
        perror("Error in ftok\n");
        return 1;
    }

    int shmid = shmget(tablekey, sizeof(int[MAX_ORDER + 1][MAX_ORDER + 1]), IPC_CREAT | 0666); // read and write both permission given
    if (shmid == -1)
    {
        perror("Error in shmget in creating/ accessing shared memory\n");
        return 1;
    }
    int(*shared_orders)[MAX_ORDER + 1];
    shared_orders = shmat(shmid, NULL, 0); // 2d array to store orders and this is basically passed to the shared segment between waiter and table
    if (shared_orders == (void *)-1)
    {
        perror("Error in shmPtr in attaching the memory segment\n");
        return 1;
    }
    shared_orders[0][1] = 0;
    shared_orders[0][5] = 0;

    printf("Enter Number of Customers at Table (maximum no. of customers can be 5): ");
    scanf("%d", &numberOfCustomer); // between 1 and 5 inclusive
    
    do
    {
        // displaying the menu to the customer
        menuRead();

        // Dynamically allocate memory for orders array
        int **orders = ordersArr(numberOfCustomer);
        // Copy orders data to shared memory
        // shared_orders[0][0] to be empty , it will either show valid order or invalid order, in case of valid order will store the bill
        shared_orders[0][0] = 0;
       // shared_orders[0][1] = numberOfCustomer; // aditya added, delete if causing trouble.
      //  printf("%d\n", shared_orders[0][1]);
        shared_orders[0][2] = ShouldWeContinue;
        for (int i = 1; i < numberOfCustomer + 1; i++)
        {
            for (int j = 1; j < MAX_ORDER + 1; j++)
            {
                shared_orders[i][j] = orders[i][j];
            }
        }
        shared_orders[0][0] = -1;
        shared_orders[0][1] = numberOfCustomer;
        shared_orders[0][4] = 1; 
        // Wait for waiter to check order validity
        while (shared_orders[0][0] == -1)
        {
        }
        // basically shared_orders[0][0] will return 2 if the orders are valid
        while (shared_orders[0][0] != 2)
        {
            shared_orders[0][0] = 0;
            // Order is invalid, prompt customers to give orders again
            printf("Invalid order detected. Please give your order again.\n\n");
            orders = ordersArr(numberOfCustomer);
            // Copy orders data to shared memory
            for (int i = 1; i < MAX_CUSTOMERS + 1; i++)
            {
                for (int j = 1; j < MAX_ORDER + 1; j++)
                {
                    shared_orders[i][j] = orders[i][j];
                }
            }
            shared_orders[0][0] = -1;
            while (shared_orders[0][0] == -1)
            {
                
                //sleep(3);
            }
        }
		while(shared_orders[0][3] == -1){
		}
        printf("Total Bill amount is %d INR.\n", shared_orders[0][3]);
		shared_orders[0][4] = 0; // order ready for waiter = 1

		// asking the table do we want more customers, end it if we get -1
        printf("Enter Number of Customers at Table (maximum no. of customers can be 5):");
        scanf("%d", &ShouldWeContinue);
        if(ShouldWeContinue>0){
            numberOfCustomer = ShouldWeContinue;
            shared_orders[0][2] = 1;
        }
        else{
            shared_orders[0][2] = ShouldWeContinue; 
        }

    } while (ShouldWeContinue != -1);
    shared_orders[0][5] = 1;
    shmdt(shared_orders);
}

// pipe creation
int **ordersArr(int numberOfCustomer)
{
    int **orders = (int **)malloc(sizeof(int *) * MAX_ORDER + 1); // 2d array
    for (int i = 0; i < MAX_ORDER + 1; i++)
    {
        orders[i] = (int *)malloc(sizeof(int) * MAX_ORDER + 1);
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
        }
        else if (pid == 0)
        {                           // child process
            close(fd[i][READ_END]); // customer is writing on the pipe, close the unused end
            int food_itm = 0;
            while (food_itm != -1)
            {
                
                printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");
                scanf("%d", &food_itm);
                write(fd[i][WRITE_END], &food_itm, sizeof(food_itm));
            }
            close(fd[i][WRITE_END]); // close write end before exiting
            exit(0);
        }
        wait(NULL);
    }
    // Table process reading the pipe
    for (int i = 0; i < numberOfCustomer; i++)
    {
        close(fd[i][WRITE_END]);
        int food_itm;
        int order_count = 1;
        // read till you encounter -1
        while (read(fd[i][READ_END], &food_itm, sizeof(food_itm))) // read from the pipe
        {
            orders[i + 1][order_count++] = food_itm; // store the order in the 2d array
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
