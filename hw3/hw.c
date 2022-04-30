#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/*
    Struct node represent linked list elements which is operated by threads. Node has 2 data fields which is
                                                                                                int data -> represents integer element
                                                                                                node next -> next element address.
*/
struct node
{
    int data;
    struct node *next;
};

/*
    Struct task_node represents queue elements in the task queue. task_node has 4 data fields which is
                                                                                                int task_num -> represent task number which is task rank
                                                                                                int task_type -> represent task type which can be one of the 3 options
                                                                                                    0 insert
                                                                                                    1 delete
                                                                                                    2 search
                                                                                                from linked list.
                                                                                                int data -> represent task data which is a value processed by task type
                                                                                                task_node next -> next task queue element address.
*/
struct task_node
{
    int task_num;
    int task_type;
    int data;
    struct task_node *next;
};

// Initiliaze queue front and rear as NULL.
// Initiliaze linked list head as NULL.
struct task_node *front = NULL;
struct task_node *rear = NULL;
struct node *head = NULL;

// Pthread variable for preventing race conditions.
pthread_mutex_t lock, lock2;
pthread_cond_t cond_var;

// Size of linked list.
int size = 0;
//
int flag = 0;
// When the main thread completes, generating tasks, it sets a global variable
int awake = 1;
// Task count after enqueuing and dequeing
int task_count = 0;
// Global variable from input from argv[]
int THREADS, TASKS;

/*
    Search function takes int value parameter and interates over linked list to fin given value is in the linked list.
*/
int search(int value)
{
    // Declare temporary node struct for iteration as a temp.
    struct node *temp;
    // Assign head in to temp
    temp = head;
    //  Iteration until temp becomes NULL
    while (temp != NULL)
    {
        // If current temp data is equal to value return 1 otherwise temp goes to the next element in to the linked list.
        if (temp->data == value)
        {
            return 1;
        }
        temp = temp->next;
    }
    // None of the temp data is equal to value return 0.
    return 0;
}

/*
    Insert function takes int value parameter and insert it to the linked list in ascending order.
*/
int insert(int value)
{
    // Initializa current node struct for inserting.
    struct node *current = malloc(sizeof(struct node));
    current->data = value;
    current->next = NULL;

    // If size is 0 it means linked list is empty and reassign head of the linked list as a curren.
    if (size == 0)
    {
        head = current;
        // Increment size of the linked list and return 1.
        size++;
        return 1;
    }

    // If value is already in the linked list return -1.
    else if (search(value))
        return -1;

    // If value is not in the linked list and and linked list contains elements.
    else
    {
        if (size == 1 || current->data < head->data) // If size of the linked list is 1 or data of the current is
        // smaller than head data first elemnt in the linked list is now on current is new head and current next contains old head address.
        {
            current->next = head;
            head = current;
            size++;
            // Increment size of the linked list and return 1.
            return 1;
        }
        else
        {
            // Now we know element size is not 1 and current data is greater than head data.
            //  Initiliaze temp node struct for linked list.
            struct node *temp = head;
            // This loop continoue until
            while (1)
            {
                // temp next is NULL it means we are at the end of the linked list.
                if (temp->next == NULL)
                    break;

                // current data is lower then next element data.
                if (current->data < temp->next->data)
                    break;

                // temp is now next element in the linked list.
                temp = temp->next;
            }
            // Now we find space for inserting.
            // Initiliaze current next same as temp next.
            current->next = temp->next;
            // temp next now is the current node address. Increment size of the linked list and return 1.
            temp->next = current;
            size++;
            return 1;
        }
    }
}

/*
    Delete function takes int value parameter and deletes it from the linked list.
*/
int delete (int value)
{
    // If size of linked list is 0 or linked list does not contains value it returns -1.
    if (size == 0 || search(value) == 0)
        return -1;

    // we now linked list is not empty and it contains value so we neeed to initialize temp node struct as head.
    struct node *temp = head;

    // If head value is the equal value we should point next element in the linked list. Returns 1.
    if (temp->data == value)
    {
        head = temp->next;
        return 1;
    }
    else
    {
        // This loop continou untill
        while (1)
        {
            // If temp is last element in the linked list.
            if (temp->next == NULL)
                break;
            // Value is equal next element of temp.
            if (value == temp->next->data)
                break;
            temp = temp->next;
        }
        // Initialize temp2 node struct.
        struct node *temp2 = temp;
        // Temp2 next is next data field of the next element of temp.
        temp2->next = temp->next->next;
        // temp is now temp2, decrement size and return 1.
        temp = temp2;
        size--;
        return 1;
    }
}

// Non-argument Display function shows element in the linked list.
void display()
{
    // Intitiliaze temp node struct. Assign head to the temp.
    struct node *temp;
    temp = head;
    printf("main:  Final list:");
    while (temp != NULL) // Loop until temp is not NULL and print data field of temp.
    {
        printf("%d\t", temp->data);
        temp = temp->next;
    }
    printf("\n");
}

/*
    task_enqueue function takes 3 paramater which is
                                        int task_num -> Task number whichs position in the task queue.
                                        int task_type -> Task type which can be
                                                            0 -> Insert
                                                            1 -> Delete
                                                            2 -> Search
                                        int data -> Data for task operation.
*/
void task_enqueue(int task_num, int task_type, int data)
{
    // Queue has FIFO approach so we nee to add element in the last postion.
    // Initiliaze temp task_node struct. And populate with parameters.
    struct task_node *temp = malloc(sizeof(struct task_node));
    temp->task_num = task_num;
    temp->task_type = task_type;
    temp->data = data;
    temp->next = NULL;
    // If task queue is empty front and rear must be point the same area.
    if (rear == NULL)
    {
        front = temp;
        rear = temp;
    }
    // If task queue is not empty last element pointed by rear so we need to add element after temp.
    else
    {
        rear->next = temp;
        rear = rear->next;
    }
    // increment task_count.
    task_count++;
}

/*
    task_dequeue function takes 4 paramenter which is
                                        long my_rank -> thread rank
                                        int *task_num_p -> task number field.
                                        int *task_type_p -> task type field. Can be
                                                            0 -> Insert
                                                            1 -> Delete
                                                            2 -> Search

                                        int *value_p -> value field.
*/
int task_dequeue(long my_rank, int *task_num_p, int *task_type_p, int *value_p)
{
    // task_node struct as temp.
    struct task_node *temp;

    // if task queue is NULL assign glob task count as 0 and return 0.
    if (front == NULL)
    {
        task_count = 0;
        return 0;
    }

    // If task queue is not empty populate temp fields.
    temp = front;
    *task_num_p = temp->task_num;
    *task_type_p = temp->task_type;
    *value_p = temp->data;
    // If queue has only one element
    if (rear = front)
    {
        rear = rear->next;
    }
    // Go next element of the front decrement task count and return 1.
    front = front->next;
    task_count--;
    return 1;
}

/*
    task_queue function takes int n for creating thread queue.
*/
void task_queue(int n)
{

    // Loop for generating random number for the task queue and enqueue it.
    for (int i = 0; i < n; i++)
    {
        // task num is current i value task type can be 0,1,2 and data is random number between 1-500.
        int task_num = i;
        int task_type = (rand() % (2 - 0 + 1)) + 0; //   number = (rand() % (upper - lower + 1)) + lower
        int data = (rand() % (500 - 1 + 1)) + 1;
        // Enqueue genarated task in to the task queue via the task_enqueue function.
        task_enqueue(task_num, task_type, data);
        // When main thread add task to the task queue it gives signal to thread to work.
        pthread_cond_signal(&cond_var);
    }
    // When the main thread completes
    // generating tasks, it sets a global variable indicating that
    // there will be no more tasks, and awakens all the threads with a condition broadcast.
    while (front != NULL)
    {
        if (front == NULL)
        {
            awake = 0;
            break;
        }
        pthread_cond_broadcast(&cond_var);
    }
    display();
    pthread_cond_broadcast(&cond_var);
}

void *procedure(void *rank)
{
    //  Loop until there is no task left via the help of globalal awake variable.
    while (awake == 1)
    {
        long my_rank = *((long *)rank); // long rank variable given by instructor.
        // First lock variable for preventing the different threads give wait signal.
        pthread_mutex_lock(&lock);
        // Wait for the signal.It provides us to stay in wait condition after executings threads.
        pthread_cond_wait(&cond_var, &lock);
        pthread_mutex_unlock(&lock);
        // If clause determines thread executed before because our purpose is multithreading if it executed.
        // If task queue is empty thread does not have to execute and code.
        if (front == NULL && flag == 1)
        {
            return NULL;
        }

        // Take a task from queue
        int task_type_p, value_p, task_num_p;
        task_dequeue(my_rank, &task_num_p, &task_type_p, &value_p);

        // Following if clause determines wheather thread do insert, delete or search.
        // In general explenation of the every if check;
        // We need to give mutex lock for prevent race conditions of the shared variables.
        // in every step we fflush stdout (Instructor Emre shows in the lab sections.)
        // We check task operation return value and print the neccesary output. Unlock the mutex.
        if (task_type_p == 0)
        {
            pthread_mutex_lock(&lock2);
            fflush(stdout);
            if (insert(value_p))
                printf("Thread %ld: task %d: %d is inserted\n",
                       my_rank,
                       task_num_p, value_p);
            else
                printf("Thread %ld: task %d: %d cannot be inserted\n",
                       my_rank,
                       task_num_p, value_p);
            pthread_mutex_unlock(&lock2);
        }
        else if (task_type_p = 1)
        {
            pthread_mutex_lock(&lock2);
            fflush(stdout);
            if (delete (value_p))

                printf("Thread %ld: task %d: %d is deleted\n",
                       my_rank,
                       task_num_p, value_p);
            else
                printf("Thread %ld: task %d: %d cannot be deleted\n",
                       my_rank,
                       task_num_p, value_p);
            pthread_mutex_unlock(&lock2);
        }
        else if (task_type_p == 2)
        {
            fflush(stdout);
            pthread_mutex_lock(&lock2);
            if (search(value_p))
                printf("Thread %ld: task %d: %d is in the list\n",
                       my_rank,
                       task_num_p, value_p);
            else
                printf("Thread %ld: task %d: %d is not in the list\n",
                       my_rank,
                       task_num_p, value_p);
            pthread_mutex_unlock(&lock2);
            break;
        }

        flag = 1;
    }
}

int main(int argc, char const *argv[])
{
    // Input error handling.
    if (argc == 3)
    {
        // begining of the clock cycle TIK TAK!!
        clock_t begin = clock();
        // takes argument from argv and set it to the global THREADS and TASKS.
        int THREADS = atoi(argv[1]);
        int TASKS = atoi(argv[2]);

        // Initiliaze threads with the size of THREADS.
        pthread_t threads[THREADS];

        // CREATE THREADS.
        for (int i = 0; i < THREADS; i++)
        {
            // We need  to keep rank of the threads so thread rank is equal to thread creatin time via the loop indicator i.
            //  pthread_create takes argument as last paramater it is int rank so in procedure we need to make int rank
            //  to long rank.
            int *rank = malloc(sizeof(int));
            *rank = i;
            pthread_create(&threads[i], NULL, &procedure, rank);
        }

        // Main thread generates tasks.
        task_queue(TASKS);

        // After creation we need to join threads.
        for (int i = 0; i < THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }

        // Destory mutex locks and conditions.
        pthread_mutex_destroy(&lock);
        pthread_mutex_destroy(&lock2);
        pthread_cond_destroy(&cond_var);
        // FFLUSH STDOUT.
        fflush(stdout);
        // TIME IS UP!
        clock_t end = clock();
        // calculate the time spent and printed it after that exit.
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Total time spent %lf, for %d threads and %d tasks.\n", time_spent, THREADS, TASKS);
        exit(0);
    }
    else if (argc > 3)
    {
        printf("Too many arguments!!\n");
    }
    else
    {
        printf("Two arguments in expected!!\n");
    }

    return 0;
}
