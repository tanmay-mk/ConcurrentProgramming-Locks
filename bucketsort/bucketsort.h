/*
 *  @fileName       :   bucketsort.h
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

#ifndef _BUCKET_SORT_H_
#define _BUCKET_SORT_H_

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <set>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <barrier>

#include "locks.h"

void bucketsort(lock_algs_t alg, barrier_types_t bar, std::vector <int>& arr, int nums, size_t numThreads);

#endif /*_BUCKET_SORT_H_*/
