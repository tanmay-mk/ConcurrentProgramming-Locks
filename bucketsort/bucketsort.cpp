/*
 *  @fileName       :   bucketsort.cpp
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

/*Include other header files*/
#include "bucketsort.h" 
#include "time.h"

/*Private defines and macros*/
#define BUCKET_DIVIDER (100)
#define TOTAL_NUMS (list_size)

using namespace std;

/*Global variables*/
int list_size;                  //total number of elements in the list
int num_buckets;                //number of buckets
static vector<thread*> threads; //vector of threads, declared static because merge_sort.cpp also has vector of threads
vector<set<int>> buckets;       //global vector of sets

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

/*Private typedefs, classes, structs and unions*/
typedef struct threadParams
{
    size_t threadId;
    int low;
    int high;
}threadParams_t;

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
 * @brief       :   worker function for each thread
 * 
 * @parameters  :   threadParams_t 
 *                      *params    :   pointer to structure containing thread params
 *                  vector <int>* 
 *                      arr         :   pointer to the array to be sorted
 *
 * @returns     :   NULL
 */
void* fillBuckets(threadParams_t* params, vector<int>* arr)
{
    int i = params->low;        //index for iterations between range
    int bkt_idx = 0;    //bucket index to store the element
    int num=0;          //element to be stored in the bucket
    Node *thisNode = new Node;  /*for MCS lock*/

    //barrier wait here 
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }
     
    //main thread records start time here
    if (params->threadId == 1)
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

    while (i <=  params->high)
    {
        num = arr->at(i);                   //get element
        bkt_idx = (int)num/num_buckets;     //determine which bucket it belongs to

        //to avoid segmentation fault
        if(bkt_idx >= num_buckets)
        {
            bkt_idx = num_buckets - 1;
        }

        //acquire lock
        switch(locktype)
        {
            case TAS_algorithm:             taslock->TAS::TAS_lock(); break;
            case TTAS_algorithm:            ttaslock->TTAS::TTAS_lock(); break;
            case TICKETLOCK_algorithm:      tktlock->TicketLock::Ticket_lock(); break;
            case MCS_algorithm:             mcslock->MCS::acquire(thisNode); break;
            case PTHREAD_algorithm:         mutexlock->lock(); break;
            case PETERSON_SEQ_algorithm:    petersonseqlock->Peterson::sequential_lock(params->threadId); break;
            case PETERSON_REL_algorithm:    petersonrellock->Peterson::released_lock(params->threadId); break;
            default:                        mutexlock->lock(); break;
        }

        //store element in the bucket
        buckets[bkt_idx].insert(num);

        //release the lock
        switch(locktype)
        {
            case TAS_algorithm:             taslock->TAS::TAS_unlock(); break;
            case TTAS_algorithm:            ttaslock->TTAS::TTAS_unlock(); break;
            case TICKETLOCK_algorithm:      tktlock->TicketLock::Ticket_unlock(); break;
            case MCS_algorithm:             mcslock->MCS::release(thisNode); break;
            case PTHREAD_algorithm:         mutexlock->unlock(); break;
            case PETERSON_SEQ_algorithm:    petersonseqlock->Peterson::sequential_unlock(params->threadId); break;
            case PETERSON_REL_algorithm:    petersonrellock->Peterson::released_unlock(params->threadId); break;
            default:                        mutexlock->unlock(); break;
        }

        //update index
        i++;
    }
    
    delete thisNode; /*delete the node for MCS lock*/

    //barrier wait here
    switch(bartype)
    {
        case PTHREAD_type:              pthreadbar->arrive_and_wait(); break;
        case SENSE_REV_type:            sensebar->Barrier::wait(); break;
        default:                        pthreadbar->arrive_and_wait(); break;
    }

    //main thread records end time here
    if (params->threadId == 1)
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


    return NULL;
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   sorts an array by spawning threads for concurrent operation
 * 
 * @parameters  :   vector <int>&
 *                      arr         :   array to be sorted
 *                  int
 *                      nums        :   number of elements in array
 *                  size_t
 *                      numThreads  :   number of threads required for concurrent operations
 *
 * @returns     :   none
 */
void bucketsort(lock_algs_t alg, barrier_types_t bar, std::vector <int>& arr, int nums, size_t numThreads)
{
    //update the list size
    list_size = nums;

    //update locking algorithm
    locktype = alg;
    bartype = bar; 

    //determine number of buckets
    if (list_size < 100)
    {
        //10 buckets are sufficient for less than 100 numbers
        num_buckets = 10;
    }
    else if (list_size > 1000)
    {
        //for high number of elements, we determine number of buckets
        //such that on average, each bucket can accommodate 100 elements
        //it is not necessary for each bucket to contain at most 100 elements, just an assumption
        num_buckets = list_size / BUCKET_DIVIDER;
    }
    else
    {
        //100 buckets are sufficient for upto 1000 elements
        num_buckets = BUCKET_DIVIDER;
    }
    
    //initialize lock
    lock_init(alg);
    barrier_init(numThreads, bar);  //initialize barrier

    //resize the buckets vector to required number of buckets
    buckets.resize(num_buckets);

    //resize the threads vector to required number of threads
    threads.resize(numThreads);

    vector<threadParams_t> params;  //vector of threadparams structure

     int numsPerThread, //total elements per thread
        low = 0,        //lower index of the range
        high = 0;       //higher index of the range
    
    //compute number of elements per thread 
    numsPerThread = list_size/numThreads;

    for (size_t i=0; i<numThreads; i++)
    {
        //check if we are populating structure for last thread, if not
        if (i != (numThreads-1))
        {
            //update the structure with computed values
            high = low + (numsPerThread - 1);
            params.push_back({  
                                i+1,    //threadId
                                low,    //lower index
                                high    //higher index
                            });
            low = high + 1; //update the low index
        }
        else
        {
            //if we are populating final structure for final thread
            //assign all remaining elements to it
            high = list_size-1;
            params.push_back({  
                                i+1,    //threadId
                                low,    //lower index
                                high    //higher index
                            });
        }
    }

    //spawn threads
    for(size_t i=1; i<numThreads; i++)
    {
        threads[i] = new thread(fillBuckets, &params[i], &arr);
    }
    //master thread will also perform
    fillBuckets(&params[0], &arr);
    
    //wait for threads to complete their execution and join them together
    for(size_t i=1; i<numThreads; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    lock_delete(alg);       /*delete locks*/
    barrier_delete(bar);    /*delete barrier*/

    //populate final array 
    int bkt_idx = 0;
    for(int i =0; i<num_buckets; i++)
    {
        for(int j:buckets[i])
        {
            arr[bkt_idx++] = j;
        }
    }
}
/*---------------------------------------------------------------------------------*/
/*EOF*/