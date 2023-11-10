// Libraries
#include <stdio.h>
#include <stdlib.h>
// pthread
#include <pthread.h>

int thread_done = 0;
int v = 0;

// Data Structure for arguments
struct Args
{
    struct Queue *q;
    int n;
    int id;
    int p;
    int c;
};

// Data Structure for Queue
struct Queue
{
    int num;
    int size;
    int *buffer;

    // Mutex
    pthread_mutex_t lock;
};

// Function to push item to queue
int pushq(int item, struct Queue *q)
{
    // Check if queue is full
    if(q->num == q->size)
    {
        return -1;
    }

    // Add item to queue
    q->buffer[q->num] = item;
    q->num++;

    return 1;
}

int popq(struct Queue *q)
{

    // Remove item from queue
    int item = q->buffer[0];

    // Shift queue
    for(int i = 0; i < q->num - 1; i++)
    {
        q->buffer[i] = q->buffer[i+1];
    }

    q->num--;

    return item;
}

// Producer function
void* producer(void* arg)
{
    // Get arguments
    struct Args *args = (struct Args *)arg;
    struct Queue *q = args->q;
    int n = args->n;
    int num;
    int check;
    int sleep_time;

    // Push n items to queue
    for(int i = 0; i < n; i++)
    {
        //num is rand(1000,9999)
        num = rand() % 9000 + 1000;

        // Lock queue
        pthread_mutex_lock(&q->lock);

        // Push item to queue
        check = pushq(num, q);
        if(check == 1) printf("Producer Thread#: %d; Item#: %d; Added Number: %d\n", args->id, i, num);
        else printf("Producer Thread#: %d; Item#: %d; Queue is full\n", args->id, i);

        // Print queue
        if(v == 1)
        {
            printf("Queue: ");
            for(int j = 0; j < q->num; j++)
            {
                printf("%d ", q->buffer[j]);
            }
            printf("\n");
        }

        // Unlock queue
        pthread_mutex_unlock(&q->lock);

        //sleep for rand(0,3) seconds
        sleep_time = rand() % 4;
        sleep(sleep_time);
    }
}

// Consumer function
void* consumer(void* arg)
{
    // Get arguments
    struct Args *args = (struct Args *)arg;
    struct Queue* q = args->q;
    int prod_num = args->n;
    float cons_num = (float)prod_num * 0.7 * args->p / (float)args->c;
    int n = (int)cons_num;
    int num;
    int check;
    int sleep_time;

    printf("Consumer Thread#: %d Initialized\n", args->id);

    // Pop n items from queue
    for(int i = 0; i < n; i++)
    {
        // Lock queue
        pthread_mutex_lock(&q->lock);
        
        // Check if queue is empty
        if(q->num == 0 && thread_done)
        {
            pthread_mutex_unlock(&q->lock);
            break;
        }
        else if(q->num == 0)
        {
            i--;
            pthread_mutex_unlock(&q->lock);
            continue;
        }

        // Pop item from queue
        num = popq(q);
        printf("Consumer Thread#: %d; Item#: %d; Removed Number: %d\n", args->id, i, num);

        // Print queue
        if(v == 1)
        {
            printf("Queue: ");
            for(int j = 0; j < q->num; j++)
            {
                printf("%d ", q->buffer[j]);
            }
            printf("\n");
        }

        // Unlock queue
        pthread_mutex_unlock(&q->lock);

        //sleep for rand(0,4) seconds
        sleep_time = rand() % 5;
        sleep(sleep_time);
    }
}


// Main function
int main(int argc, char *argv[])
{
    // p - prod threads, c - cons threads, s - buffer size, n - number of items
    int p = 4, c = 2, s = 5, n = 5;
    // Input arguments with tags
    for(int i = 0;i < argc; i++)
    {
        if(argv[i] == '-')
        {
            if(argv[i+1] == 'p')
            {
                p = atoi(argv[i+2]);
            }
            else if(argv[i+1] == 'c')
            {
                c = atoi(argv[i+2]);
            }
            else if(argv[i+1] == 's')
            {
                s = atoi(argv[i+2]);
            }
            else if(argv[i+1] == 'n')
            {
                n = atoi(argv[i+2]);
            }
            else if(argv[i+1] == 'v')
            {
                v = 1;
            }
        }
    }

    // Create queue
    struct Queue *q = (struct Queue *)malloc(sizeof(struct Queue));
    q->num = 0;
    q->size = s;
    q->buffer = (int *)malloc(s * sizeof(int));

    // Create threads
    pthread_t prod[p];
    pthread_t cons[c];
    thread_done = 0;

    // Initiate cons threads
    for(int i = 0; i < c; i++)
    {
        // Create arguments
        struct Args *args = (struct Args *)malloc(sizeof(struct Args));
        args->q = q;
        args->n = n;
        args->id = i;
        args->p = p;
        args->c = c;

        // Create thread
        pthread_create(&cons[i], NULL, consumer, (void *)args);
    }

    sleep(1);

    // Initiate prod threads
    for(int i = 0; i < p; i++)
    {
        // Create arguments
        struct Args *args = (struct Args *)malloc(sizeof(struct Args));
        args->q = q;
        args->n = n;
        args->id = i;
        args->p = p;
        args->c = c;

        // Create thread
        pthread_create(&prod[i], NULL, producer, (void *)args);
    }    

    // Join all prod threads
    for(int i = 0; i < p; i++)
    {
        pthread_join(prod[i], NULL);
    }

    // Notify cons threads that prod threads are done
    pthread_mutex_lock(&q->lock);
    thread_done = 1;
    pthread_mutex_unlock(&q->lock);

    // Join all cons threads
    for(int i = 0; i < c; i++)
    {
        pthread_join(cons[i], NULL);
    }

    printf("Program is done\n");
}