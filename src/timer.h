/**
 * timer.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef TIMER_H
#define TIMER_H

#include <ostream>
#include <sys/times.h>
#include <sys/types.h>
#include "common.h"

namespace vorpal::nibac {
/***
 * This class mimicks a stopwatch: it provides functionality to start timing, stop timing,
 * and reset. It is printable using standard ostream techniques.
 */
    class Timer {
    private:
        struct tms tp;
        unsigned long cstart, cstop;
        double seconds;

    public:
        Timer() = default;
        virtual ~Timer() = default;

        /**
         * The start method tells the Timer to begin recording time elapsed.
         */
        void start(void);

        /**
         * The stop method tells the Timer to stop recording time elapsed.
         */
        void stop(void);

        /**
         * The reset method simply sets the number of seconds elapsed back to 0.
         */
        void reset(void);

        /**
         * The setSeconds method is used to begin the Timer at a certain point, if
         * such behaviour is ever desired.
         */
        void setSeconds(double);

        /**
         * This method returns the number of seconds for which the Timer has been active
         * (i.e. the number of seconds elapsed cumulatively between calls to start and stop).
         */
        double getSeconds(void) const;
    };

    /**
    * For a Timer, this method outputs the number of seconds elapsed with two decimal
    * points of precision, in the format: "xxxx.xx s".
    */
    std::ostream &operator<<(std::ostream &, const Timer &);
};
#endif
