/**
 * @file ClockTimer.h
 * @brief Class for computing elapsed time, starting and stopping a virtual
 * stopwatch, and checking whether a time limit has been reached
 *
 * This version uses gettimeofday(), so it's "wall clock" time, which is okay
 * unless there's heavy I/O or other processes running
 *
 * @author Matt Stallmann
 * @date 2019/05/17
 */

#ifndef CLOCKTIMER_H
#define CLOCKTIMER_H

#include<iostream>
#include<sys/time.h>

const double MILLION = 1e6;

/// Usage:
///   ClockTimer my_timer = ClockTimer()
///   my_timer.start();
///   ... run first thing
///   my_timer.stop()
///   ... run second thing
///   my_timer.start();
///   ... run third thing
///   my_timer.stop()
///   ... at this point getTotalTime() returns the total time for running
///   first and third thing   
class ClockTimer {
public:
    ClockTimer() { reset(); }
    void reset() { elapsed_time.tv_sec = 0; elapsed_time.tv_usec = 0; }
    void start() {
        gettimeofday(&current_time, NULL);
        time_at_last_start = current_time;
    }
    void stop() {
        gettimeofday(&current_time, NULL);
        elapsed_time.tv_sec += current_time.tv_sec - time_at_last_start.tv_sec;
        elapsed_time.tv_usec += current_time.tv_usec - time_at_last_start.tv_usec;
    }
    double getTotalTime() const {
        return elapsed_time.tv_sec + elapsed_time.tv_usec / MILLION;
    }
    void setTimeLimit(double seconds) {
        time_limit = seconds;
    }
    bool timeLimitReached() const {
        if ( getTotalTime() >= time_limit ) return true;
        return false;
    }
private:
    double time_limit;
    struct timeval elapsed_time;
    struct timeval current_time;
    struct timeval time_at_last_start;
};

#endif

// Local Variables: ***
//  mode:c++ ***
// End: ***

//  [Last modified: 2019 05 17 at 20:21:26 GMT]
