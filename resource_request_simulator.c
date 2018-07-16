#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

//declaring the semaphores
sem_t full;
sem_t empty;
//mutual exclusion
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//the shared variable
int **max;
int *avail;
int *req;
int **need;
int **hold;
//number of processes
int numProcesses;
//number of resources
int numResources;

/*
Simulates resource requests by processes
*/
void request_simulator(int pr_id, int* request_vector){
    //initializing request vector to a random value between and 0 and its need
    //critical section
    printf("Requesting resources for process %d\n", pr_id);
    for(int i=0; i < numResources;i++){
        request_vector[i] = (rand() % (need[pr_id][i] + 1));
    }    
    //printing the resource vector
    printf("The resource vector requested array is: ");
    for(int i=0; i < numResources;i++){
        printf("%d ", request_vector[i]);
    }
}
/*
Implementation of isSafe() as described in the slides
*/
int isSafe(){
    //step1: initialize
    int isSafe = 0;
    int* finish = malloc(sizeof(int) *numProcesses);
    int* work = malloc(sizeof(int) * numResources);
    for(int j=0;j<numResources;j++) work[j] = avail[j];
    for(int i=0;i<numProcesses;i++) finish[i] = 0;
step2: //find a process Pi such that finish = false and need[i][j] <= work[j]
    for(int i=0;i<numProcesses;i++){
        if(finish[i] == 0){
            int flag = 1;
            for(int j=0;j<numResources;j++){
                if(need[i][j] > work[j]) flag = 0;
            }
            
            if(flag != 0){
                //step3:
                //critical section
                for(int j=0;j<numResources;j++){
                    work[j] = work[j] + hold[i][j];
                }
                finish[i] = 1;
                goto step2;
            }
        }
    }
//step4:
    for(int i =0;i<numProcesses;i++){
        if(finish[i] == 0) {
            isSafe = 0;//the system is not safe
            return isSafe;
        }
    }
    //the system is safe
    isSafe = 1;
    return isSafe;
}
/*
Implementation of Bankers Algorithm as described in the slides
returns 1 if safe allocation 0 if not safe
*/
int bankers_algorithm(int pr_id, int* request_vector){
// step1: Verify that a process matches its need. if it doesnt exit(1)
    for(int j=0;j<numResources;j++){
        if(request_vector[j] > need[pr_id][j]){
            printf("Error, Impossible\n");
            exit(1);
	    }
    
        //step2: Check if the requested amount is available. if not, then the process must wait
    
        if(request_vector[j] > avail[j]) continue;
    
        //step3: provisional allocation
        //critical section 
    
        avail[j] = avail[j] - request_vector[j];
        hold[pr_id][j] = hold[pr_id][j] + request_vector[j];
        need[pr_id][j] = need[pr_id][j] - request_vector[j];

        //check if the system is safe after the allocation
        if(isSafe()){
            printf("System is safe: allocating resource %d to process %d\n",j,pr_id);
            //grant resources -- system is safe
        }
        else{
            printf("System is not safe, cancelling allocation for resource %d to process %d\n",j,pr_id);
            //critical section
            avail[j] = avail[j] + request_vector[j];
            hold[pr_id][j] = hold[pr_id][j] - request_vector[j];
            need[pr_id][j] = need[pr_id][j] + request_vector[j];
        }
    }
    return 0;

}

/*
Simulates processes running on the system.

*/
void* process_simulator(void* pr_id){
    //get the id of the process
    int id = *(int *) pr_id;
    int done = 1;
    while(done){
        done = 0;
        pthread_mutex_lock(&mutex);
        //call request_simulator to get a random request vector
        printf("Requesting resources for process %d\n", id);
        request_simulator(id,req);

        printf("\nChecking if allocation is safe\n");
        //calling banker's algorithm to check if the allocation is safe or not
        bankers_algorithm(id,req);
        
        //here it is safe. we now check if the process can terminate
        for(int i=0;i<numResources;i++){
            if(need[id][i] != 0) {
                done = 1;
                break;
            }
        }
        if(done == 0) break;
        pthread_mutex_unlock(&mutex);
        //sleep for 3 seconds and make another request. i.e. call process_simulator again
        sleep(3);
    }
    //free allocation
    printf("--------------------------------------------\n");
    printf("Process %d has finished. Releasing resources\n", id);
    printf("--------------------------------------------\n");

    
    for(int i=0;i<numResources;i++){
        //critical section
        avail[i] += hold[id][i];
        hold[id][i] -= hold[id][i];
        need[id][i] = -1;
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

/*
Simulates a fault occuring on the system.

*/
void* fault_simulator(void* pr_id){
    
}
/*
Checks for deadlock
*/
void* deadlock_checker(void * pr_id){
    
}

int main(){

    //reading the input from user
    printf("Please specify the number processes:\n");
    scanf("%d",&numProcesses);

    printf("Please specify the number of resources:\n");
    scanf("%d",&numResources);

    //1D array of available resources
    avail = malloc(sizeof(int)*numResources);
    req = malloc(sizeof(int)*numResources);

    //array of threads(processes)
    pthread_t processThreads[numProcesses];
    pthread_t fault;
    pthread_t deadlock;


    //integer pointer to array of processes ID
    int *processIDs[numProcesses];
    //reading input from user
    printf("please specify the amount of each resource in the system has:\n");
    for(int i=0;i<numResources;i++){
        scanf("%d",&avail[i]);
    }
    
    //2D array of maximum resources each process can claim 
    //Initialize all inputs to banker's algorithm
    need = malloc(numResources*sizeof(int*));
    hold = malloc(numResources*sizeof(int*));
    max = malloc(numResources*sizeof(int*));
    for(int i=0;i<numProcesses;i++){
        max[i] = (int *) malloc(sizeof(int)*numResources);
        hold[i] = (int *) malloc(sizeof(int)*numResources);
        need[i] = (int *) malloc(sizeof(int)*numResources);
    }

    //reading the input of max from user
    printf("please specify the maximum resources each process can claim:\n");
    for(int i=0;i<numProcesses;i++){
        for(int j=0;j<numResources;j++){
            scanf("%d",&max[i][j]);
        }
    }
    //initializing need to have the same amount as max and hold to zero
    for(int i=0;i<numProcesses;i++){
        for(int j=0;j<numResources;j++){
            need[i][j] = max[i][j];
            hold[i][j] = 0;
        }
    }

    printf("The Number of each resource in the system is: ");
    for(int i=0; i< numResources;i++) printf("%d ",avail[i]);

    printf("\nThe Allocated Resources table is:\n");
    for(int i=0; i< numProcesses;i++){
        for(int j=0;j<numResources;j++){
            printf("%d ",hold[i][j]);
        }
        printf("\n");
    }
    printf("The Maximum Claim table is:\n");
    for(int i=0; i< numProcesses;i++){
        for(int j=0;j<numResources;j++){
            printf("%d ",max[i][j]);
        }
        printf("\n");
    }
    printf("Simulating process simulator\n");
    //create threads simulating processes (process_simulator)
    for(int i=0;i<numProcesses;i++){
        processIDs[i] = malloc(sizeof(int));
        *processIDs[i] = i;
        pthread_create(&processThreads[i],NULL,process_simulator,processIDs[i]);
    }
    //create a thread that takes away resources from the available pool (fault_simulator)

    //create a thread to check for deadlock (deadlock_checker)

    //joining threads
    for(int j=0;j<numProcesses;j++) pthread_join(processThreads[j],NULL);


    
    //destroying all the semaphores
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
    return 0;
}