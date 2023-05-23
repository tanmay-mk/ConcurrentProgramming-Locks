/*
 *  @fileName       :   counter.h
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */
#ifndef _COUNTER_H_
#define _COUNTER_H_

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <barrier>

#include "locks.h"

int counter(lock_algs_t alg, barrier_types_t bar, bool barFlag, int num, size_t numThreads);

#endif /*_COUNTER_H_*/