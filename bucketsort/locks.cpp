/*
 *  @fileName       :   locks.cpp
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

/*LIBRARY FILES*/
#include "locks.h"

using namespace std; 

/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Aqcuires the lock using TAS method
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void TAS::TAS_lock()
{
    bool expected = false;
    while(!taslock.compare_exchange_strong(expected, true, SEQ_CST))
    {
        expected = false;
    }
}

/*
 * @brief       :   Releases the lock using TAS method
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void TAS::TAS_unlock()
{
    taslock.store(false, SEQ_CST);
} 
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Aqcuires the lock using TTAS method
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void TTAS::TTAS_lock()
{
    bool expected = false;
    while(  
            (ttaslock.load(SEQ_CST) == true) || 
            !(ttaslock.compare_exchange_strong(expected, true, SEQ_CST))
        )
    {
        expected = false;
    }
}

/*
 * @brief       :   Releases the lock using TTAS method
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void TTAS::TTAS_unlock()
{
    ttaslock.store(false, SEQ_CST);
} 
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Aqcuires the lock using Ticket lock method
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void TicketLock::Ticket_lock()
{
    int my_num = next_num.fetch_add(1, SEQ_CST);
	while (now_serving.load(SEQ_CST) != my_num);
}

/*
 * @brief       :   Releases the lock using ticket lock method
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void TicketLock::Ticket_unlock()
{
    now_serving.fetch_add(1, SEQ_CST);
} 
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Aqcuires the lock using MCS lock method
 *
 * @params      :   Node
 *                      *nodeptr    :   pointer to current thread's node
 * 
 * @returns     :   None
 */
void MCS::acquire(Node *nodeptr)
{
    Node *oldTail = tail.load(SEQ_CST);

    nodeptr->next.store(NULL, RELAXED);

    while(!tail.compare_exchange_strong(oldTail, nodeptr, SEQ_CST))
    {
        oldTail = tail.load(SEQ_CST);
    }

    //if oldTail == NULL, we have acquired the lock
    //otherwise, wait for it

    if(oldTail != NULL)
    {
        nodeptr->wait.store(true, RELAXED);
        oldTail->next.store(nodeptr, SEQ_CST);
        while(nodeptr->wait.load(SEQ_CST));
    }

}

/*
 * @brief       :   Releases the lock using MCS lock method
 *
 * @params      :   Node
 *                      *nodeptr    :   pointer to current thread's node
 * 
 * @returns     :   None
 */
void MCS::release(Node *nodeptr)
{
    Node *n = nodeptr; 

    if(tail.compare_exchange_strong(n, NULL, SEQ_CST))
    {
        //no one is waiting, we just freed the lock
    }
    else
    {
        while(nodeptr->next.load(SEQ_CST) == NULL);
        nodeptr->next.load(SEQ_CST)->wait.store(false, SEQ_CST);
    }
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Implements a barrier using sense reversal barrier algorithm
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void Barrier::wait()
{
    thread_local bool mySense = false; 

    if (false == mySense)   //flip sense here
    {
        mySense = true;
    }
    else
    {
        mySense = false; 
    }

    int cnt_cpy = count.fetch_add(1, SEQ_CST);
    if (cnt_cpy == (numThreads-1))  //last to arrive
    {
        count.store(0, RELAXED);
        sense.store(mySense, SEQ_CST);
    }
    else    //not last
    {
        while(sense.load(SEQ_CST) != mySense);
    }
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Aqcuires the lock using peterson's algorithm using sequential
 *                  consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void Peterson::sequential_lock(int threadId)
{
    threadId--;
    int myTid; 
    int otherTid;
    if (threadId == 0)
    {
        myTid = 0;
        otherTid = 1;
    }
    else
    {
        myTid = 1;
        otherTid = 0; 
    }

    //say you want to acquire lock
    desires[myTid].store(true, SEQ_CST);

    //but first, give other thread the chance to acquire lock
    turn.store(otherTid, SEQ_CST);

    //wait here until the other thread loses the desire
    //to acquire the lock or it is your turn to get the lock
    while((desires[otherTid].load(SEQ_CST)) && (turn.load(SEQ_CST)==otherTid));
}

/*
 * @brief       :   Releases the lock using peterson's algorithm using sequential
 *                  consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void Peterson::sequential_unlock(int threadId)
{
    threadId--;
    int myTid; 
    if (threadId == 0)
    {
        myTid = 0;
    }
    else
    {
        myTid = 1;
    }
    //you do not desire to acquire lock
    //allowing other thread to acquire the lock
    desires[myTid].store(false, SEQ_CST);
}
/*---------------------------------------------------------------------------------*/
/*
 * @brief       :   Aqcuires the lock using peterson's algorithm using released
 *                  consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void Peterson::released_lock(int threadId) 
{
    threadId--;
    int myTid; 
    int otherTid;
    if (threadId == 0)
    {
        myTid = 0;
        otherTid = 1;
    }
    else
    {
        myTid = 1;
        otherTid = 0; 
    }

    //say you want to acquire lock
    desires[myTid].store(true, RELEASE);

    //but first, give other thread the chance to acquire lock
    turn.store(otherTid, SEQ_CST);

    //wait here until the other thread loses the desire
    //to acquire the lock or it is your turn to get the lock
    while((desires[otherTid].load(RELEASE)) && (turn.load(SEQ_CST)==otherTid));
}

/*
 * @brief       :   Releases the lock using peterson's algorithm using released
 *                  consistency
 *
 * @params      :   None
 * 
 * @returns     :   None
 */
void Peterson::released_unlock(int threadId) 
{
    threadId--;
    int myTid; 
    if (threadId == 0)
    {
        myTid = 0;
    }
    else
    {
        myTid = 1;
    }
    //you do not desire to acquire lock
    //allowing other thread to acquire the lock
    desires[myTid].store(false, SEQ_CST);
}
/*---------------------------------------------------------------------------------*/
/*EOF*/