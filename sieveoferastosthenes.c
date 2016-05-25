#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

struct thread_data
{
    int	thread_id;
    int offset;
    int end_point;
};

typedef struct {
    pthread_mutex_t		lock;		/* mutex semaphore for the barrier */
    pthread_cond_t		ok_to_proceed;	/* condition variable for leaving */
    int				     threads_completed;		/* count of the number who have arrived */
} barrier_t;


int N, NUM_THREADS, currentPrime = 2, *primes;
barrier_t barrier;
FILE *file;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;


void barrier_init(barrier_t* b){
    pthread_mutex_init(&(b->lock), NULL);
    pthread_cond_init(&(b->ok_to_proceed), NULL);
    b->threads_completed = 0;
}

void getNextPrime(){
    int i = currentPrime;
    for ( i = i+1; i < N; i++) {
        if(primes[i]==0){
            currentPrime = i;
            return;
        }
    }
}

void* syncPoint(barrier_t *b){
    
    pthread_mutex_lock(&b->lock);
    b->threads_completed++;
    
    if(b->threads_completed<NUM_THREADS){
        printf("waiting threads %d\n",b->threads_completed);
        pthread_cond_wait(&b->ok_to_proceed, &b->lock);
    }
    
    else if(b->threads_completed==NUM_THREADS){
        printf("all threads done\n");
        b->threads_completed = 0;
        getNextPrime();
        pthread_cond_broadcast(&b->ok_to_proceed);
    }
    
    pthread_mutex_unlock(&b->lock);
    
    return NULL;
}

void *getPrimes(void *threadarg)
{
    struct thread_data *my_data = (struct thread_data*)threadarg;
    int startAt = my_data->offset;
    int stopAt = my_data->end_point;
    
    //continue while current prime number is less than SQRT of N (max value)
    while (currentPrime < sqrt(N)) {
    for (int x = startAt ; x <= stopAt; x++) {
        
        if(x%currentPrime == 0 && primes[x]== 0 && currentPrime!=x){
                primes[x] = -1;
        }
    }
        if(NUM_THREADS>1){
            syncPoint(&barrier);
        }else{
            currentPrime++;
        }
        
    }
    
    pthread_exit(NULL);
}

    


int main(int argc, char *argv[])
{
    
    if(argc<3){
        printf("Argument parse error**.\n\tmain <number> <threads>\n");
        return -1;
    }
    
    if(atoi(argv[2])<1){
        printf("Invalid number of threads\n");
        return -1;
    }
    
    N = atoi(argv[1]);
    
    if(N<2){
        printf("N MUST be > 2\n");
        return -1;
    }
    
    NUM_THREADS = atoi(argv[2]);
    struct thread_data thread_data_array[NUM_THREADS];
    
    primes = malloc(sizeof(int) * N);
    
    //initialize barrier
    barrier_init(&barrier);
    
    //initialize array to all 0's
    for (int i = 0 ; i < N; i++)
        primes[i]=0;
    
    pthread_t threads[NUM_THREADS];
    int rc;
    
    
    for(int t = 0;t < NUM_THREADS;t++) {
        thread_data_array[t].thread_id = t;
        
        if(t==0)
            thread_data_array[t].offset = 2;
        else
            thread_data_array[t].offset = t*N/NUM_THREADS+1;
        
        thread_data_array[t].end_point = (t+1)*N/NUM_THREADS;
        
        rc = pthread_create(&threads[t],NULL,getPrimes,(void*) &thread_data_array[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    //join
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    
    file = fopen("primes.txt", "w");
    int count_primes=0;
    
    printf("prime numbers between %d and %d\n\n",2,N);
    fprintf(file, "prime numbers between %d and %d\n\n",2,N);
    for (int i = 2 ; i <= N; i++) {
        if(primes[i] == 0){
            fprintf(file, "%d ",i);
            printf("%d ",i);
            count_primes++;
        }
    }
    
    printf("\n\n%d prime number(s) found\n\n",count_primes);
    fprintf(file,"\n\n%d prime number(s) found\n\n",count_primes);
 
    fclose(file);
    
    //return 0;
    
    
    pthread_exit(NULL);
}
