/*
This project is the source code to solve the Burger Buddies Problem

Filename: BurgerBuddies.c

Purpose: Simulate the Burger Buddies Problem by printing the current situation.

This program maintain a queue to store the current situation of sleeping cashiers, and mainly use mutex and semaphore to control the threads:
            mutex:
                cntMutex: the mutex of count numbers
                checkRack: the mutex of rack full check
                queueMutex: the mutex of the queue
                mutex1( 2, 3): only one cook( customer or cashier) doing at the sametime
            semaphore:
                finish: check the serveredcustomer, decide the end time
                rackFull(rackRmpty): maintain the current burgers' number
                nocashier: the current cashier number

*/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
pthread_mutex_t cntMutex, checkRack, queueMutex;//define mutex
pthread_mutex_t mutex1, mutex2, mutex3;
sem_t *cashierAvailable;//define semaphore for every cashier
pthread_t *Cooks, *Cashiers, *Customers;//define thread 
sem_t finish, rackFull, rackEmpty, nocashier;//define semaphore
int rack = 0, Servedcustomers= 0, waitForCashier = 0, waitForBurger = 0;
int head = 0, tail = -1;
int cooks, cashiers, customers, racksize;
int *sleepingQueue;

int queueSize()//the necessary queue function
{
    return ((tail + 1 - head + cashiers + 1) % (cashiers + 1));
}
void enqueue(int id)
{
    tail = (tail + 1) % (cashiers + 1);
    sleepingQueue[tail] = id;
}
int dequeue()
{
	int res = 0;
    if (head != (tail + 1) % (cashiers + 1))
    {
    	res = sleepingQueue[head];
    	head = (head + 1) % (cashiers + 1);
    }
    return res;
}
void *cook(void *param)//cook thread 
{
	int id = *(int*)param; // get ID
    
    do
    {
    	pthread_mutex_lock(&mutex1); 
    	pthread_mutex_lock(&checkRack); 
        
        if (rack == racksize) //rack full 
        {
        	pthread_mutex_unlock(&checkRack); 
        	sem_wait(&rackFull); 
            pthread_mutex_lock(&checkRack);
        }

        ++rack; // add a burger to the rack
	
	    printf("Cook [%d] makes a burger.\n", id);
        if (rack == 1 && waitForBurger) //the customer take a burger away
        {
        	--waitForBurger;
        	sem_post(&rackEmpty);
        }
        pthread_mutex_unlock(&checkRack); 

        pthread_mutex_unlock(&mutex1);
	    sleep(1);//cook time
  
    } while (1); 
}
void *cashier(void *param)
{
	int id = *(int*)param; // get ID

    do
    {
    	pthread_mutex_lock(&queueMutex);
        enqueue(id); // add to the sleeping queue

        if (queueSize() == 1 && waitForCashier) // if there are customers waiting for vacant cashier
        {
        	--waitForCashier;
        	sem_post(&nocashier); 
        }
        pthread_mutex_unlock(&queueMutex);

    	sem_wait(&cashierAvailable[id-1]); // get ordered
	    printf("Cashier [%d] accepts an order.\n", id);

        pthread_mutex_lock(&mutex2); // only one cashier to get the burger
	    pthread_mutex_lock(&checkRack);

        if (rack == 0) // no burgers
        {
	        ++waitForBurger;
	        pthread_mutex_unlock(&checkRack); 
	        sem_wait(&rackEmpty); 
	        pthread_mutex_lock(&checkRack);
        }

	    --rack; // get a burger

	    if (rack + 1 == racksize) sem_post(&rackFull);
	    pthread_mutex_unlock(&checkRack);
        printf("Cashier [%d] takes a burger to customer.\n", id);
	    pthread_mutex_unlock(&mutex2);	

        pthread_mutex_lock(&cntMutex);
        ++ Servedcustomers; // customers served, used to end up the process

        if (Servedcustomers== customers) sem_post(&finish); // finish serve
        pthread_mutex_unlock(&cntMutex);
    } while (1);
}
void *customer(void *param)
{
	int id = *(int*)param;//get id
	sleep(1);
	pthread_mutex_lock(&mutex3); //one customer

    pthread_mutex_lock(&queueMutex);
    int tmp = queueSize();//get size

    if (tmp == 0) //first come
    {
    	++waitForCashier; 
        pthread_mutex_unlock(&queueMutex);
    	sem_wait(&nocashier); 
        pthread_mutex_lock(&queueMutex);
    }
	printf("Customer [%d] comes.\n", id);

    int serverID = dequeue(); 
    pthread_mutex_unlock(&queueMutex);
    sem_post(&cashierAvailable[serverID-1]); 
	pthread_mutex_unlock(&mutex3);

	pthread_exit(0); //customer exit
}
void *finishserve(void *param)
{
	sem_wait(&finish); //wait for finish
    int i;
    for (i = 0; i < cooks; ++i) pthread_kill(Cooks[i], 0); //kill thread 
    for (i = 0; i < cashiers; ++i) pthread_kill(Cashiers[i], 0); 
    pthread_exit(0);
}
int main(int argc,char ** argv)
{	

    int i, id;
    if (argc != 5) // exception handling
    {
    	printf("Please check you input!\n");
    	printf("The correct format should be:\n");
    	printf("./BBC #Cooksnum #Cashiersnum #Customersnum #RackSizenum\n");
    	return 0;
    }
    cooks = atoi(argv[1]);
    cashiers = atoi(argv[2]);
    customers = atoi(argv[3]);
    racksize = atoi(argv[4]);
    sleepingQueue = (int *)malloc((cashiers + 1) * sizeof(int));
    printf("Cooks [%d], Cashiers [%d], Customers [%d]\n", cooks, cashiers, customers);
    printf("Begin run.\n");

    int max = cooks;
    if (max < cashiers) max = cashiers;
    if (max < customers) max = customers;
    int *numbers = (int *)malloc((max + 1) * sizeof(int));
    for (i = 0; i <= max; ++i) numbers[i] = i;
    
    // mutexes & semaphores initialzation
    pthread_mutex_init(&cntMutex, NULL);
    sem_init(&finish, 0, 0);
    pthread_mutex_init(&queueMutex, NULL);
    pthread_mutex_init(&checkRack, NULL);
    sem_init(&rackEmpty, 0, 0);
    sem_init(&rackFull, 0, 0);
    sem_init(&nocashier, 0, 0);
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_mutex_init(&mutex3, NULL);
    cashierAvailable = (sem_t *)malloc(cashiers * sizeof(sem_t));
    for (i = 0; i < cashiers; ++i)
        sem_init(&cashierAvailable[i], 0, 0);

    // cook threads
    Cooks = (pthread_t *)malloc(cooks * sizeof(pthread_t));
    for (i = 0; i < cooks; ++i)
    {
    	pthread_create(&Cooks[i], NULL, cook, (void *)&numbers[i+1]);
    }

    // cashier threads
    Cashiers = (pthread_t *)malloc(cashiers * sizeof(pthread_t));
    for (i = 0; i < cashiers; ++i)
    {
    	pthread_create(&Cashiers[i], NULL, cashier, (void *)&numbers[i+1]);
    }

    // customer threads
    Customers = (pthread_t *)malloc(customers * sizeof(pthread_t));
    for (i = 0; i < customers; ++i)
    {
    	pthread_create(&Customers[i], NULL, customer, (void *)&numbers[i+1]);
    }

    //check finish threads;
    pthread_t Finishserve;
    pthread_create(&Finishserve, NULL, finishserve, NULL);
    pthread_join(Finishserve, NULL);

    return 0;
   
}

