/*
19CS10014 Ayush Pattnayak
19CS10072 Pramit Chandra
*/

#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <queue>

using namespace std;

#define MAX_QUEUE_SIZE 8
#define ELEM_MOD 19
#define MAX_MAT_ID 100000
#define MATRIX_DIM 1000
#define MAX_EXTRA_SIZE 1000

// structure to represent job
typedef struct job{
	
    int prod_no;
    int mat_id;
    int mult_stat;
    int matrix[MATRIX_DIM][MATRIX_DIM];
	int comp_time;
    pid_t prod_pid;
    pid_t prod_proc_id;
}job;

//structure to represent shared memory
typedef struct shared_memory{
	job q[MAX_QUEUE_SIZE];
    int size,front;
	int job_created;
	int job_completed;
	int num_mat;
	int computed;
	sem_t mutex;
	sem_t full;
	sem_t empty;
}shared_memory;

shared_memory* init_shm(int shmid,int num_mat);
void producer(int shmid,int prod_no,pid_t prod_pid);
void worker(int shmid,int cons_no,pid_t cons_pid);
job create_job(pid_t prod_pid,int prod_no);
void print_job(job j);
void insert_job(shared_memory* shm,job j);
job remove_job(shared_memory* shm);
void local_mult(job *j1,job *j2,job *j3);

int main(){

	// starting point for generating random numbers for parent process
	srand(time(0));
	// input
	int NP,NC,num_mat;
	cout<<"Enter no of Producers:\n";
	cin>>NP;
    cout<<"Enter no of Workers:\n";
	cin>>NC;
	cout<<"Enter Max possible matrices: \n";
	cin>>num_mat;

	//array to hold producer pids
	pid_t prods[NP];
	//array to hold worker pids
	pid_t conss[NC];

	// generate a unique key using ftok
	key_t key = ftok("/dev/random",'c');
	// create shared memory
	int shmid = shmget(key,sizeof(shared_memory),0666|IPC_CREAT);
	if(shmid<0){
		cout<<"Errno=%d\n",errno;
		cout<<"Error in creating shared memory. Exiting..\n";
		exit(1);
	}
	// attach and initialize the shared memory
	shared_memory* shm = init_shm(shmid,num_mat);

	time_t start = time(0);

	pid_t pid;
	
	for(int i=1;i<=NP;i++)
	{
		pid=fork();
		if(pid<0){
			cout<<"Error in creating producer process. Exiting..\n";
			exit(1);
		}
		else if(pid==0)// in producer process
		{
			// different seed for each producer process
			srand(time(0)+i);
			int prod_pid=getpid();
			producer(shmid,i,prod_pid);
			
			return 0;
		}
		else
		{
			//include the producer in prods array to kill later
			prods[i-1]=pid;
		}
	}
	// create all workers
	for(int i=1;i<=NC;i++)
	{
		pid=fork();
		if(pid<0){
			cout<<"Error in creating worker process. Exiting..\n";
			exit(1);
		}
		else if(pid==0)// in worker process
		{
			srand(time(0)+NP+i);
			int cons_pid=getpid();
			worker(shmid,i,cons_pid);
			
			return 0;
		}
		else
		{
			//include the worker in consss array to kill later
			conss[i-1]=pid;
		}
	}

	// loop till all jobs are created and consumed
	while(1){
		sem_wait(&(shm->mutex));
		
		if(shm->job_created>=num_mat && shm->job_completed>=num_mat && shm->size==1 &&shm->computed>=num_mat )
		{
			time_t end = time(0);
			int time_taken = end-start;
			cout<<"Time taken to run %d jobs= %d seconds\n",num_mat,time_taken;
			// kill all child processes
			for(int i=0;i<NP;i++)
				kill(prods[i],SIGTERM);
			for(int i=0;i<NC;i++)
				kill(conss[i],SIGTERM);
			sem_post(&(shm->mutex));
			break;		
		}
		sem_post(&(shm->mutex));
	}
	//destroy mutex semaphore
	sem_destroy(&(shm->mutex));
	
	shmdt(shm);//detach shared memory segment
	shmctl(shmid,IPC_RMID,0);//mark shared memory segment to be destroyed
	return 0;
}

// attaches shared memory to a memory segment
// and initializes the data structure members
shared_memory* init_shm(int shmid,int num_mat)
{
	shared_memory* shm = (shared_memory*)shmat(shmid,NULL,0);
	shm->size = 0;
    shm->front = 0;
	shm->num_mat = num_mat;
	shm->job_created = 0;
	shm->job_completed = 0;
	shm->computed = 0;
    
    

	// initialize the semaphore mutex
	//binary semaphore for access to jobs_created, jobs_completed, insertion & retrieval of jobs
	int sema = sem_init(&(shm->mutex),1,1);
	//counting semaphore to check if the q is full
	int full_sema = sem_init(&(shm->full),1,0);
	//counting semaphore to check if the q is empty
	int empty_sema= sem_init(&(shm->empty),1,MAX_QUEUE_SIZE);
	if(sema<0||full_sema<0||empty_sema<0){
		cout<<"Error in initializing semaphore. Exitting..\n";
		exit(1);
	}
	return shm;
}
// producer function
void producer(int shmid,int prod_no,pid_t prod_pid)
{
	shared_memory* shmp=(shared_memory*)shmat(shmid,NULL,0);
	while(1)
	{
		// if all jobs created, exit
		if(shmp->job_created==shmp->num_mat)
			break;
		// create job
		job j = create_job(prod_pid,prod_no);
		// random delay between 0-3 sec
        int delay_time = rand()%4;
		sleep(delay_time);
		// wait for empty semaphore
		sem_wait(&(shmp->empty));
		// wait for mutex
		sem_wait(&(shmp->mutex));
		// if all jobs created, exit
		if(shmp->job_created==shmp->num_mat)
		{
			sem_post(&(shmp->mutex));
			break;
		}
		if(shmp->size < MAX_QUEUE_SIZE)
		{
			insert_job(shmp,j);
			cout<<"Produced job details:\n";
			print_job(j);
			// increment shared variable
			shmp->job_created++;
			// signal the full semaphore		
			sem_post(&(shmp->full));						
		}
		// signal mutex
		sem_post(&(shmp->mutex));
	}
	// detach this process from shared memory
	shmdt(shmp);
}
// worker function
void worker(int shmid,int cons_no,pid_t cons_pid)
{
	shared_memory* shmc=(shared_memory*)shmat(shmid,NULL,0);
	while(1)
	{
		// random delay
		sleep(rand()%4);
		// wait for the full semaphore
		sem_wait(&(shmc->full));
		// wait to acquire mutex
		sem_wait(&(shmc->mutex));
		job j1,j2;
		// flag to indicate a job is retrieved
		int job_retrieved=0;
		
		// signal mutex
		sem_post(&(shmc->mutex));

		if(shmc->size>=2)
		{
			if(shmc->q[0].mult_stat!=shmc->q[1].mult_stat){
                cout<<"Multiplication error occured\n";
                exit(-1);
            }
            job * j1 = &(shmc->q[shmc->size-1]),*j2 = &(shmc->q[shmc->size-2]);
            
            if(j1->mult_stat==0){
                job j = create_job(cons_pid,cons_no);
                job * jptr = &j;
                jptr->prod_pid = cons_pid;
                j1->prod_proc_id = cons_pid;
                j2->prod_proc_id = cons_pid;
                local_mult(j1,j2,jptr);
                insert_job(shmc,*jptr);
                printf("Worker job details\n");
			    printf("Worker: %d,",cons_no);
			    printf("Worker pid: %d,",cons_pid);
                cout<<"Inserted job in queue: \n";
                print_job(*jptr);
            }
            else{
                pid_t check = j1->prod_proc_id;
                int ind=0;
                job *j;
                while(ind<shmc->size){
                    if(shmc->q[ind].prod_pid==check && shmc->q[ind].mult_stat==0){
                        j = &(shmc->q[ind]);
                        break;
                    }
                    ind++;
                }
                if(ind==shmc->size){
                    cout<<"No incomplete product found in queue.\n";
                }

                if(j1->mult_stat==8 && j2->mult_stat==8){
                    remove_job(shmc);
                    remove_job(shmc);
                }
                printf("Worker job details\n");
			    printf("Worker: %d,",cons_no);
			    printf("Worker pid: %d,",cons_pid);
                cout<<"Edited job in queue: \n";
                print_job(*j);
                    
            }

			sem_wait(&(shmc->mutex));
			// increment shared variable
			shmc->job_completed++;
			// signal mutex
			sem_post(&(shmc->mutex));
			// signal empty semaphore
			sem_post(&(shmc->empty));
			
			// to ensure worker is killed only after it has slept/computed job
			sem_wait(&(shmc->mutex));
			shmc->computed++;
			sem_post(&(shmc->mutex));
		}
        else if(shmc->size<=1){
            //do nothing
        }
	}
	// detach from shared memory
	shmdt(shmc);
}
// create a job taking producer process ID and number as input
// and using random integers for comp_time & mat_id
job create_job(pid_t prod_pid,int prod_no)
{
	job j;
	j.prod_pid = prod_pid;
	j.prod_no = prod_no;
	j.comp_time = rand()%4+1;
	j.mat_id = rand()%MAX_MAT_ID+1;
    j.mult_stat = 0;
    j.prod_proc_id = prod_pid;

    for(int i=0;i<MATRIX_DIM;i++){
        for(int k=0;k<MATRIX_DIM;k++){
            j.matrix[i][k] = rand()%(ELEM_MOD)-9;
        }
    }

	return j;
}


// insert a new job in the the job queue
void insert_job(shared_memory* shm,job j)
{
	if(shm->size==MAX_QUEUE_SIZE)
	{
		cout<<"Overflow, insertion not possinle\n";
		return;
	}
	
    shm->q[shm->size] = j;
    shm->size++;

}
// pop a job from the queue
job remove_job(shared_memory* shm)
{
	if(shm->size==0)
	{
		job j;
		j.mult_stat=-1;
		return j;
	}

	job root = shm->q[0];
	for(int ind=0;ind< shm->size - 1;ind++){
        shm->q[ind] = shm->q[ind+1];
    }
    shm->size--;
	return root;
}
// print the details of a job
void print_job(job j)
{
    cout<<"Producer pid: "<<j.prod_pid;
    cout<<"Job ID: "<<j.mat_id;
    cout<<"Producer no: "<<j.prod_no;
    cout<<"Compute time"<<j.comp_time;
    cout<<"Matrix status "<<j.mult_stat;

}

void local_mult(job *j1,job *j2,job *j3){
    int block_count = j1->mult_stat;
    if(block_count!=j2->mult_stat){
        cout<<"Invalid multiplication occurred\n";
    }
    int k = block_count/2,x;
    block_count = block_count%4;
    int row=(block_count/2)*MATRIX_DIM,col=(block_count%2)*MATRIX_DIM;

    for(int i=row;i<row + (MATRIX_DIM/2);i++){
        for(int j=col;j<col+(MATRIX_DIM)/2;j++){
            x=0;
            for(int t=0;t<(MATRIX_DIM)/2;t++) 
                x+=j1->matrix[i][t]*j2->matrix[t][j];
            if(k){
                j3->matrix[i][j] += x;
            }
            else j3->matrix[i][j] = x;
        }
    }
    j1->mult_stat++;
    j2->mult_stat++;
    j3->mult_stat++;
    return;
    
}
