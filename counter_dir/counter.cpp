/*
 *  @fileName       :   counter.cpp
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

/*LIBRARY FILES*/
#include "counter.h"
#include "time.h"

using namespace std; 

/*GLOBAL VARIABLES */
static vector<thread*> threads;         /*vector of threads*/
int ctr = 0;                            /*global counter to be incremented*/
int numIterations = 0;                  /*number of times each thread will increment the counter*/
int num_threads=4;                      /*number of threads, 4 by default*/   

TAS *taslock;                           /*pointer to TAS lock implementation class*/
TTAS *ttaslock;                         /*pointer to TTAS lock implementation class*/
TicketLock *tktlock;                    /*pointer to Ticket lock implementation class*/
MCS *mcslock;                           /*pointer to MCS lock implementation class*/
mutex *mutexlock;                       /*pointer to Mutex lock implementation class*/
Peterson *petersonseqlock;              /*pointer to Peterson lock implementation class, used for sequential consistency*/
Peterson *petersonrellock;              /*pointer to Peterson lock implementation class, used for released consistency*/
Barrier *sensebar;                      /*pointer to sense reversal barrier implementation class*/
barrier<> *pthreadbar;                  /*pointer to pthread barrier implementation class*/

lock_algs_t locktype = PTHREAD_algorithm;   /*locking algorithm to be used, mutex default*/
barrier_types_t bartype = PTHREAD_type;     /*barrier type to be used, pthread barrier default*/

/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to TAS lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void TAS_init()
{
    //we will lock each bucket individually
    taslock = new TAS;
    taslock->taslock.store(false, SEQ_CST);
}

/*
 * @brief       :   Deletes the pointer to TAS lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void TAS_delete()
{
    delete taslock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to TTAS lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void TTAS_init()
{
    ttaslock = new TTAS;
    ttaslock->ttaslock.store(false, SEQ_CST);
}

/*
 * @brief       :   Deletes the pointer to TTAS lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void TTAS_delete()
{
    delete ttaslock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to Ticket lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void TicketLock_init()
{
    tktlock = new TicketLock;
    tktlock->next_num.store(0, SEQ_CST);
    tktlock->now_serving.store(0, SEQ_CST);
}

/*
 * @brief       :   Deletes the pointer to Ticket lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void TicketLock_delete()
{
    delete tktlock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to MCS lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void MCS_init()
{
    mcslock = new MCS;
    mcslock->tail.store(NULL, SEQ_CST);
}

/*
 * @brief       :   Deletes the pointer to MCS lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void MCS_delete()
{
    delete mcslock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to mutex lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void mutex_init()
{
    mutexlock = new mutex;
}
/*
 * @brief       :   Deletes the pointer to mutex lock implementation
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void mutex_delete()
{
    delete mutexlock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to peterson lock implementation with
 *                  sequential consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void petersonSeq_init()
{
    petersonseqlock = new Peterson;
    petersonseqlock->desires[0].store(false, SEQ_CST);
    petersonseqlock->desires[1].store(false, SEQ_CST);
    petersonseqlock->turn.store(0, SEQ_CST);
}

/*
 * @brief       :   Deletes the pointer to peterson lock implementation with
 *                  sequential consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void petersonSeq_delete()
{
    delete petersonseqlock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initializes the pointer to peterson lock implementation with
 *                  released consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void petersonRel_init()
{
    petersonrellock = new Peterson;
    petersonrellock->desires[0].store(false, SEQ_CST);
    petersonrellock->desires[1].store(false, SEQ_CST);
    petersonrellock->turn.store(0, SEQ_CST);
}

/*
 * @brief       :   Deletes the pointer to peterson lock implementation with
 *                  released consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void petersonRel_delete()
{
    delete petersonrellock;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initialize lock
 *
 * @params      :   lock_algs_t 
 *                      alg     :   Locking algorithm to be used
 * 
 * @returns     :   None
 */
static void lock_init(lock_algs_t alg)
{
    switch(locktype)
    {
        case TAS_algorithm:             TAS_init(); break;
        case TTAS_algorithm:            TTAS_init(); break;
        case TICKETLOCK_algorithm:      TicketLock_init(); break;
        case MCS_algorithm:             MCS_init(); break;
        case PTHREAD_algorithm:         mutex_init(); break;
        case PETERSON_SEQ_algorithm:    petersonSeq_init(); break;
        case PETERSON_REL_algorithm:    petersonRel_init(); break;
        default:                        mutex_init(); break; 
    }
}
/*
 * @brief       :   Delete lock
 *
 * @params      :   lock_algs_t 
 *                      alg     :   Locking algorithm to be used
 * 
 * @returns     :   None
 */
static void lock_delete(lock_algs_t alg)
{
    switch(locktype)
    {
        case TAS_algorithm:             TAS_delete(); break;
        case TTAS_algorithm:            TTAS_delete(); break;
        case TICKETLOCK_algorithm:      TicketLock_delete(); break;
        case MCS_algorithm:             MCS_delete(); break;
        case PTHREAD_algorithm:         mutex_delete(); break;
        case PETERSON_SEQ_algorithm:    petersonSeq_delete(); break;
        case PETERSON_REL_algorithm:    petersonRel_delete(); break;
        default:                        mutex_delete(); break; 
    }
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initialize pthread barrier
 *
 * @params      :   size_t 
 *                      num_threads     :   Number of threads
 * 
 * @returns     :   None
 */
static void pthread_barrier_init(size_t num_threads)
{
    pthreadbar = new barrier(num_threads);
}

/*
 * @brief       :   Delete pthread barrier
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void pthread_barrier_delete()
{
    delete pthreadbar;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initialize sense reversal barrier
 *
 * @params      :   size_t 
 *                      num_threads     :   Number of threads
 * 
 * @returns     :   None
 */
static void sense_barrier_init(size_t num_threads)
{
    sensebar = new Barrier;
    sensebar->count.store(0, SEQ_CST);
    sensebar->sense.store(false, SEQ_CST);
    sensebar->numThreads = (int) num_threads; 
}

/*
 * @brief       :   Delete sense reversal barrier
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
static void sense_barrier_delete()
{
    delete sensebar;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Initialize barrier
 *
 * @params      :   size_t 
 *                      num_threads     :   Number of threads
 *                  barrier_types_t
 *                      bar             :   type of barrier
 * 
 * @returns     :   None
 */
static void barrier_init(size_t num_threads, barrier_types_t bar)
{
    switch(bar)
    {
        case PTHREAD_type:              pthread_barrier_init(num_threads); break;
        case SENSE_REV_type:            sense_barrier_init(num_threads); break;
        default:                        pthread_barrier_init(num_threads); break;
    }
}
/*
 * @brief       :   Delete barrier
 *
 * @params      :   barrier_types_t
 *                      bar             :   type of barrier
 * 
 * @returns     :   None
 */
static void barrier_delete(barrier_types_t bar)
{
    switch(bar)
    {
        case PTHREAD_type:              pthread_barrier_delete(); break;
        case SENSE_REV_type:            sense_barrier_delete(); break;
        default:                        pthread_barrier_delete(); break;
    }
}

/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Worker thread that increments counter variable using locks
 *
 * @params      :   size_t
 *                      threadId    :   Unique id of thread
 * 
 * @returns     :   NULL
 */
void *counter_lock(size_t threadId)
{
    Node *thisNode = new Node; /*for MCS lock only*/

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    //main thread records start time here
    if (threadId == 1)
    {
        getTime(&startTime);
    }

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    /*increment counter*/
    for (int i = 0; i<numIterations; i++)
    {
        //acquire lock
        switch(locktype)
        {
            case TAS_algorithm:             taslock->TAS::TAS_lock(); break;
            case TTAS_algorithm:            ttaslock->TTAS::TTAS_lock(); break;
            case TICKETLOCK_algorithm:      tktlock->TicketLock::Ticket_lock(); break;
            case MCS_algorithm:             mcslock->MCS::acquire(thisNode); break;
            case PTHREAD_algorithm:         mutexlock->lock(); break;
            case PETERSON_SEQ_algorithm:    petersonseqlock->Peterson::sequential_lock(threadId); break;
            case PETERSON_REL_algorithm:    petersonrellock->Peterson::released_lock(threadId); break;
            default:                        mutexlock->lock(); break;
        }

        //update counter
        ctr++;

        //release the lock
        switch(locktype)
        {
            case TAS_algorithm:             taslock->TAS::TAS_unlock(); break;
            case TTAS_algorithm:            ttaslock->TTAS::TTAS_unlock(); break;
            case TICKETLOCK_algorithm:      tktlock->TicketLock::Ticket_unlock(); break;
            case MCS_algorithm:             mcslock->MCS::release(thisNode); break;
            case PTHREAD_algorithm:         mutexlock->unlock(); break;
            case PETERSON_SEQ_algorithm:    petersonseqlock->Peterson::sequential_unlock(threadId); break;
            case PETERSON_REL_algorithm:    petersonrellock->Peterson::released_unlock(threadId); break;
            default:                        mutexlock->unlock(); break;
        }
    }

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    //main thread records stop time here
    if (threadId == 1)
    {
        getTime(&endTime);
    }

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    delete thisNode; /*delete the node for MCS*/

    /*work done, return*/
    return NULL; 
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Worker thread that increments counter variable using barrier
 *
 * @params      :   size_t
 *                      threadId    :   Unique id of thread
 * 
 * @returns     :   NULL
 */
void *counter_barrier(size_t threadId)
{
    
    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    //main thread records start time here
    if (threadId == 1)
    {
        getTime(&startTime);
    }

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    threadId--; // adjust to zero based tid's

    /*increment counter*/
    for(int i = 0; i<numIterations*num_threads; i++)
    {
        /*check if it's your turn*/
		if((i%num_threads)==threadId)
        {
            //if it's your turn, update counter and go to barrier
			ctr++;
		}
		
        /*otherwise wait here*/
        switch(bartype)
        {
            case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
            case SENSE_REV_type:            sensebar->Barrier::wait(); break;
            default:                        pthreadbar->arrive_and_wait(); break;
        } 
	}
    
    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }    

    threadId++; //restore original thread id's

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    //main thread records end time here
    if (threadId == 1)
    {
        getTime(&endTime);
    }

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    /*work done, return*/
    return NULL; 
}
/*---------------------------------------------------------------------------------*/\
/*
 * @brief       :   Worker thread that increments counter variable using barrier
 *
 * @params      :   lock_algs_t
 *                      alg         :   Locking algorithm to be used
 *                  barrier_types_t
 *                      bar         :   Barrier to be used
 *                  bool
 *                      barFlag     :   whether to use barrier implementation or not
 *                  int 
 *                      num         :   Number of iterations
 *                  size_t
 *                      numThreads  :   Number of threads to be used
 * 
 * @returns     :   int
 *                      final count updated by all threads
 */
int counter(lock_algs_t alg, barrier_types_t bar, bool barFlag, int num, size_t numThreads)
{
    locktype = alg;             /*update lock type*/
    bartype = bar;              /*update barrier type*/
    numIterations = num;        /*update number of iterations*/
    num_threads = numThreads;   /*update number of threads*/
    threads.resize(numThreads); /*resize the threads vector*/

    lock_init(alg);             /*initialize lock*/
    barrier_init(numThreads, bar);  /*initialize barrier*/
    
    if (barFlag == true)
    {
        //we will increment counter using barrier synchronization
        for(size_t i=1; i<numThreads; i++)
        {
            threads[i] = new thread(counter_barrier, i+1);
        }
        //master thread will also perform
        counter_barrier(1);
    }
    else
    {
        //we will increment counter using locks
        for(size_t i=1; i<numThreads; i++)
        {
            threads[i] = new thread(counter_lock, i+1);
        }
        //master thread will also perform
        counter_lock(1);
    }

    //wait for threads to complete their execution and join them together
    for(size_t i=1; i<numThreads; i++)
    {
        threads[i]->join();
        delete threads[i];
    }
    
    lock_delete(alg);           /*delete lock*/
    barrier_delete(bar);        /*delete barrier*/

    //counter value should be (numThreads*numIterations)
    return ctr;                 /*return updated count*/
}
/*---------------------------------------------------------------------------------*/
/*EOF*/