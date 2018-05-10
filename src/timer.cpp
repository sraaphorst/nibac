/**
 * timer.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <limits.h>
#include <sys/times.h>
#include "common.h"
#include "timer.h"

namespace vorpal::nibac {
    void Timer::start() {
        times(&tp);
        cstart = tp.tms_utime;
    }


    void Timer::stop() {
        if (cstart != ULONG_MAX) {
            times(&tp);
            cstop = tp.tms_utime;
            seconds += (((double) (cstop - cstart)) / CLK_TCK);
            cstart = ULONG_MAX;
        }
    }


    void Timer::reset() {
        seconds = 0;
    }


    void Timer::setSeconds(double nseconds) {
        seconds = nseconds;
    }


    double Timer::getSeconds() const {
        // If the timer is running, we adjust to return the number
        // of seconds it has been running for so that we can
        // get intermediate (i.e. non-stopped) results.
        if (cstart != ULONG_MAX) {
            struct tms tp2;
            times(&tp2);
            return ((double) (tp2.tms_utime - cstart)) / CLK_TCK;
        }
        return seconds;
    }


    std::ostream &operator<<(std::ostream &out, const Timer &t) {
        out << t.getSeconds() << " s";
        return out;
    }
};