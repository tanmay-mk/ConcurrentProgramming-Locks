/*
 *  @fileName       :   locks.h
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

#ifndef _LOCKS_H_
#define _LOCKS_H_

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <cstdbool>

/*GLOBAL DEFINES*/
#define SEQ_CST (std::memory_order_seq_cst)     /*sequential consistency*/
#define RELEASE (std::memory_order_acq_rel)     /*release consistency*/
#define RELAXED (std::memory_order_relaxed)     /*relaxed consistency*/

/*ENUMERATED LIST OF ALL AVAILABLE LOCKING ALGORITHMS*/
typedef enum locking_algorithms
{
    None = -1,
    TAS_algorithm = 0, 
    TTAS_algorithm = 1, 
    TICKETLOCK_algorithm = 2,
    MCS_algorithm = 3,
    PTHREAD_algorithm = 4,
    PETERSON_SEQ_algorithm = 5,
    PETERSON_REL_algorithm = 6   
}lock_algs_t;

/*ENUMERATED LIST OF ALL AVAILABLE BARRIER ALGORITHMS*/
typedef enum barrier_types
{
    SENSE_REV_type = 0, 
    PTHREAD_type 
}barrier_types_t;

/*TAS lock class definition*/
class TAS 
{
    public:
        std::atomic<bool> taslock;

        void TAS_lock();    //defined in locks.cpp

        void TAS_unlock();  //defined in locks.cpp
};

/*TTAS lock class definition*/
class TTAS
{
    public:
        std::atomic<bool> ttaslock;

        void TTAS_lock();   //defined in locks.cpp

        void TTAS_unlock(); //defined in locks.cpp
};

/*Ticket Lock class definition*/
class TicketLock
{
    public:
        std::atomic<int> next_num;
        std::atomic<int> now_serving;

        void Ticket_lock(); //defined in locks.cpp

        void Ticket_unlock();   //defined in locks.cpp
};  

/*Node class for MCS lock definition*/
class Node
{
    public:
        std::atomic<Node*> next;
        std::atomic<bool> wait;
};

/*MCS lock class definition*/
class MCS
{
    public: 
        std::atomic<Node*> tail;

        void acquire(Node *nodeptr);    //defined in locks.cpp

        void release(Node *nodeptr);    //defined in locks.cpp

};

/*Sense reversal barrier class definition*/
class Barrier
{
    public: 
        std::atomic<int> count;
        std::atomic<bool> sense;
        int numThreads;

        void wait();    //defined in locks.cpp
};

/*Peterson algorithm class definition*/
class Peterson
{
    public: 
        std::atomic<bool> desires[2];
        std::atomic<int> turn;

        void sequential_lock(int threadId); //defined in locks.cpp
        void sequential_unlock(int threadId);   //defined in locks.cpp
        void released_lock(int threadId);   //defined in locks.cpp
        void released_unlock(int threadId); //defined in locks.cpp  
};  

#endif /*_LOCKS_H_*/