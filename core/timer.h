// timer.h
//
// By Sebastian Raaphorst, 2003.
//
// A timer used to track the number of seconds
// in CPU time that a procedure requires.
// This class has been modified to be more like an
// actual stopwatch in that it can be started, stopped,
// restarted, restopped, etc... and the time changes
// are cumulative.

#ifndef TIMER_H
#define TIMER_H

#include <ostream>
#include <sys/times.h>
#include <sys/types.h>
#include "common.h"

/// A class used for measuring time.
/***
 * This class mimicks a stopwatch: it provides functionality to start timing, stop timing,
 * and reset. It is printable using standard ostream techniques.
 */
class Timer
{
 private:
  struct tms tp;
  unsigned long cstart, cstop;
  double seconds;

 public:
  /// Default constructor.
  /***
   * The default constructor simply creates a new Timer and resets it.
   */
  Timer();

  /// Default destructor.
  virtual ~Timer();

  /// Begin timing.
  /***
   * The start method tells the Timer to begin recording time elapsed.
   */
  void start(void);

  /// Stop timing.
  /***
   * The stop method tells the Timer to stop recording time elapsed.
   */
  void stop(void);

  /// Reset the timer.
  /***
   * The reset method simply sets the number of seconds elapsed back to 0.
   */
  void reset(void);

  /// Set the amount of time elapsed.
  /***
   * The setSeconds method is used to begin the Timer at a certain point, if
   * such behaviour is ever desired.
   */
  void setSeconds(double);

  /// Returns the number of recorded seconds elapsed.
  /***
   * This method returns the number of seconds for which the Timer has been active
   * (i.e. the number of seconds elapsed cumulatively between calls to start and stop).
   */
  double getSeconds(void) const;
};

/// Output mechanism for Timer class.
/***
 * For a Timer, this method outputs the number of seconds elapsed with two decimal
 * points of precision, in the format: "xxxx.xx s".
 */
std::ostream &operator<<(std::ostream&, const Timer&);

#endif
