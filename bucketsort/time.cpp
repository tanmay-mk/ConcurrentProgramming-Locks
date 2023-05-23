/*
 *  @fileName       :   time.cpp
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

#include "time.h"

struct timespec startTime, endTime;

/*
 * @brief       :   record monotonic time in a structure
 * 
 * @parameters  :   struct timespec 
 *                      *dest  :   pointer to structure where recorded
 *                                  time is to be stored
 *
 * @returns     :   none
 */
void getTime(struct timespec *dest)
{
    clock_gettime(CLOCK_MONOTONIC, dest);
}

/*
 * @brief       :   prints difference between time recorded in two sructs 
 * 
 * @parameters  :   none
 *
 * @returns     :   none
 */
void printTimeDifference()
{
    unsigned long long elapsed_ns;
    elapsed_ns = (endTime.tv_sec-startTime.tv_sec)*1000000000 + (endTime.tv_nsec-startTime.tv_nsec);
    printf("Elapsed (ns): %llu\n",elapsed_ns);
    double elapsed_s = ((double)elapsed_ns)/1000000000.0;
    printf("Elapsed (s): %lf\n",elapsed_s);
}

/*EOF*/