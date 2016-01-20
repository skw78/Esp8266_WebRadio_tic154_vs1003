/*
 * $Id: u_time.h,v 1.10 2015/07/18 00:10:52 anoncvs Exp $
 *
 * Defines for local timet_to_tm() and tm_to_timet() functions (replacements
 * for the SDK gmtime() and mktime() functions, which eat all of the ESP8266
 * RAM when called).
 *
 * ----------------------------<ATTRIBUTION>-------------------------------
 * This file is based on the C++ Arduino Playground Time.h library, originally
 * authored by "mem".  I hope "mem" will accept my apologies for the terrible
 * mess I've made of his original code.  Gomen-ne!
 * --------------------------<END ATTRIBUTION>-------------------------------
 *
 */

#ifndef __U_TIME_H__
#define __U_TIME_H__

#include <time.h>

/* 
 * Constants.
 */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL)	// The time_t value at the very start of Y2K.


/*
static const char* wdays[]=
{
  "Su",
  "Mo",
  "Tu",
  "We",
  "Th",
  "Fr",
  "Sa"

};
*/

static const char* wdays[]=
{
  "Âñ",
  "Ïí",
  "Âò",
  "Ñð",
  "×ò",
  "Ïò",
  "Ñá"

};


/* 
 * Days per month.
 * Note that the standard time.h starts months from "1", but this 
 * version starts from "0".
 */
static	const uint8_t monthDays[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
 * Standard "struct tm" equivalent.
 * =NOTE= Differences:-
 * 	- Year offset is from 1970 (standard tm  is from 1900).
 * 	- Wday Sunday is day "1" (standard tm has it as day "0").
 * 	- The "Day" field is what standard tm refers to as "yday".
 * 	- There is no "date" or Day-of-Month field.
 * 	- There is no "isdst" field.
 */
struct tmElements { 
	uint8_t Second; 
	uint8_t Minute; 
	uint8_t Hour; 
	uint8_t Wday;		// Day of week, with Sunday as day 1.
	uint8_t Day;		// This is "yday" - Day of Year.
	uint8_t Month; 
	uint8_t Year;		// Offset from 1970.
};


// This leap year calulator expects year argument as years offset from 1970.
#define LEAP_YEAR(Y)		 ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )


/*
 * Convenience macros to convert to and from tm years.
 */ 
#define  tmYearToCalendar(Y) ((Y) + 1970)	// Full, four-digit year.
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)		// Offset from 2000.
#define  y2kYearToTm(Y)      ((Y) + 30)   


/* 
 * Useful Macros for getting elapsed time values.
 */
#define numberOfSeconds(_time_)		(_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_)		((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_)		(( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_)		((( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)+1)	// 1 = Sunday.
#define elapsedDays(_time_)		( _time_ / SECS_PER_DAY)			// The number of days since Jan 1 1970.
#define elapsedSecsToday(_time_)	(_time_ % SECS_PER_DAY)				// The number of seconds since last midnight.


/*
 * The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971.
 * Always set the correct time before setting alarms.
 */
#define previousMidnight(_time_)	(( _time_ / SECS_PER_DAY) * SECS_PER_DAY)	// Time at the start of the given day.
#define nextMidnight(_time_)		( previousMidnight(_time_)  + SECS_PER_DAY )	// Time at the end of the given day.
#define elapsedSecsThisWeek(_time_)	(elapsedSecsToday(_time_) +  ((dayOfWeek(_time_)-1) * SECS_PER_DAY) )	// Week starts on day 1.
#define previousSunday(_time_)		(_time_ - elapsedSecsThisWeek(_time_))		// Time at the start of the week for the given time.
#define nextSunday(_time_)		( previousSunday(_time_)+SECS_PER_WEEK)		// Time at the end of the week for the given time.


/* 
 * Useful Macros for converting elapsed time to a time_t value.
 */
#define minutesToTime_t	((M))		( (M) * SECS_PER_MIN)  
#define hoursToTime_t	((H))		( (H) * SECS_PER_HOUR)  
#define daysToTime_t	((D))		( (D) * SECS_PER_DAY)	// Fixed on Jul 22 2011.
#define weeksToTime_t	((W))		( (W) * SECS_PER_WEEK)   


/*
 * Time and date function defines.
 * Low-level functions to convert to and from system time.
 */
void	timet_to_tm(time_t time, struct tmElements *tmel);	// Convert a time_t value into "struct tm" elements.
time_t	tm_to_timet(struct tmElements *tmel);			// Reconstitute "struct tm" elements into a time_t value.

#endif
