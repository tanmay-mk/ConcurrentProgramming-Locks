/*
 *  @fileName       :   time.h
 *
 *  @author         :   tanmay-mk
 *
 *  @date           :   26 Oct 2022
 *                                           
 */

#ifndef _TIME_H_
#define _TIME_H_

#include <iostream>
#include <ctime>

extern struct timespec startTime, endTime;

void getTime(struct timespec *dest);

void printTimeDifference();

#endif /*_TIME_H_*/