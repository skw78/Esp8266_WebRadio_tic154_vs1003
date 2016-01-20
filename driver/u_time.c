/*
 * $Id: u_time.c,v 1.6 2015/07/18 00:10:52 anoncvs Exp $
 *
 * Replacement functions for gmtime() and mktime() from the SDK (which deplete
 * ESP8266 memory so severely that compiles fail when they are used).
 *
 * ----------------------------<ATTRIBUTION>-------------------------------
 * This file is based on the C++ Arduino Playground Time.h library, originally
 * authored by "mem".  I hope "mem" will accept my apologies for the terrible
 * mess I've made of his original code.  Gomen-ne!
 * --------------------------<END ATTRIBUTION>-------------------------------
 *
 */
#include <c_types.h>
#include <time.h>
#include "u_time.h"

/*
 * Convert the "timeInput" time_t count into "struct tm" time components.
 * This is a more compact version of the C library localtime function.
 * Note that year is offset from 1970 !!!
 */
void timet_to_tm(time_t timeInput, struct tmElements *tmel){

	uint8_t year;
	uint8_t month, monthLength;
	uint32_t time;
	unsigned long days;

	time = (uint32_t)timeInput;
	tmel->Second = time % 60;
	time /= 60;		// Now it is minutes.
	tmel->Minute = time % 60;
	time /= 60;		// Now it is hours.
	tmel->Hour = time % 24;
	time /= 24;		// Now it is days.
	tmel->Wday = ((time + 4) % 7) + 1;	// Sunday is day 1.
	
	year = 0;	
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
		year++;
	}
	tmel->Year = year;	// The year is offset from 1970.
	
	days -= LEAP_YEAR(year) ? 366 : 365;
	time	-= days;	// Now it is days in this year, starting at 0.
	
	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++) {
		if (month==1) {	// February.
			if (LEAP_YEAR(year)) {
				monthLength=29;
			} else {
				monthLength=28;
			}
		} else {
			monthLength = monthDays[month];
		}
		
		if (time >= monthLength) {
			time -= monthLength;
		} else {
				break;
		}
	}
	tmel->Month = month + 1;	// Jan is month 1.
	tmel->Day = time + 1;		// Day of month.
}

/*
 * Reconstitute "struct tm" elements into a time_t count value.
 * Note that the year argument is offset from 1970.
 */
time_t tm_to_timet(struct tmElements *tmel){	 
	
	int i;
	uint32_t seconds;

	// Seconds from 1970 till 1st Jan 00:00:00 of the given year.
	seconds= tmel->Year*(SECS_PER_DAY * 365);
	for (i = 0; i < tmel->Year; i++) {
		if (LEAP_YEAR(i)) {
			seconds += SECS_PER_DAY;	// Add extra days for leap years.
		}
	}
	
	// Add the number of elapsed days for the given year. Months start from 1.
	for (i = 1; i < tmel->Month; i++) {
		if ( (i == 2) && LEAP_YEAR(tmel->Year)) { 
			seconds += SECS_PER_DAY * 29;
		} else {
			seconds += SECS_PER_DAY * monthDays[i-1];	// "monthDay" array starts from 0.
		}
	}
	seconds+= (tmel->Day-1) * SECS_PER_DAY;		// Days...
	seconds+= tmel->Hour * SECS_PER_HOUR;		// Hours...
	seconds+= tmel->Minute * SECS_PER_MIN;		// Minutes...
	seconds+= tmel->Second;				// ...and finally, Seconds.
	return (time_t)seconds; 
}
