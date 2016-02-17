#include <ccRTC.h>

rtcClass ccRTC;

const int Const_Month_Array[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const int Const_Weekday_Lookday[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

rtcClass::rtcClass(void){
    Timezone = 0;
    Timezoneadjust = 0;
    Minute = 0;
    Hour = 0;
    Day = 0;
    Weekday = 0;
    Month = 0;
    Year = 0;
    Beginstatus = false;
    Leapyearflag = false;
    for(int i=0; i<12; i++){
        Month_Array[i] = Const_Month_Array[i];
        Weekday_Lookup[i] = Const_Weekday_Lookday[i];
    }
}


void rtcClass::begin(void){
    PRCMRTCInUseSet();
    PRCMRTCSet(0, 0);
    Beginstatus = true;
}


void rtcClass::zone(float timezone){
    if((timezone >= -12) && (timezone <= 12)){
        Timezone = timezone;
        Timezoneadjust = (int) (3600 * Timezone);
    }
}


void rtcClass::setTime(unsigned long newtime){
    PRCMRTCSet(newtime, 0);
}


unsigned long rtcClass::now(void){
    unsigned long temp_seconds = 0;
    unsigned short temp_mseconds = 0;
    PRCMRTCGet(&temp_seconds, &temp_mseconds);
    //temp_seconds += temp_timestamp    // Removing this since it is a bug. Timestamp is universal and need not be adjusted by Timezone
    return temp_seconds;
}


int rtcClass::claculateWeekday(int y, int m, int d){
    if(m < 3){
        y = y - 1;
    }
    else{
        y = y;
    }
    return ((y + y/4 - y/100 + y/400 + Weekday_Lookup[m-1] + d) % 7) + 1;
}


void rtcClass::updateTimeVariables(unsigned long temptime){
    int temp_timestamp = temptime;
    int total_days;
    int total_t_years;
    int total_leap_years;
    int total_days_leap_adjust;
    int total_years;
    int days_left_in_year;
    int seconds_in_day;
    int minutes_in_day;

    /* Adding Time Zone Adjustment to the timestamp to calculate the Year, Month, Day, etc for that particular Timezone*/
    temp_timestamp += Timezoneadjust;

    /* This is to make sure timestamp is not negative.
    Ideally negative seconds can be converted to time, but it requires some foolproof logic which is currently not implemented */
    if(temp_timestamp < 0){
        temp_timestamp = 0;
    }

    /* total_days is immune to leap year adjustment.*/
    total_days = (int) (temp_timestamp / 86400);

    /* total_years is not immune to leap year adjustment.
    Hence a temporary variable total_t_years is used first.
    total_t_years is an approximate number of years since 1970. */
    total_t_years = (int) (total_days / 365);

    /* total_leap_years will have the number leap years that have already gone by.
    So if current year is 2 (1970 + 2), there should be 0 leap years gone by. Therefore (2 + 1)/4 = 0.
    But if current year is 3 (1970 + 3), there should be 1 leap year gone by which is 2 (1972). Therefore (3 + 1)/4 = 1*/
    total_leap_years = (int) ((total_t_years + 1) / 4);

    /* total_days_leap_adjust will have all the excess leap year days removed from it.
    This is to make sure that when total_days_leap_adjust is divided by 365, it won't be hindered by the excess leap years*/
    total_days_leap_adjust = total_days - total_leap_years;

    /* total_years will have the number of years since 1970 */
    total_years = (int) (total_days_leap_adjust / 365);

    /* Year will have the current year in the form of YYYY */
    Year = total_years + 1970;

    /* Set the leap year flag for later use and also adjust the number of days in February accordingly */
    if ((Year % 4) == 0){
        Leapyearflag = true;
        Month_Array[1] = 29;
    }
    else{
        Leapyearflag = false;
        Month_Array[1] = 28;
    }

    /* days_left_in_year will have the number of days over with in the current year minus 'today'
    (THIS HAS ONE ISSUE WHICH IS FIXED IN THE NEXT LINE */
    days_left_in_year = (int) (total_days_leap_adjust - (total_years * 365));

    /* Unfortunately this is more like TREATING THE SYMPTOMS.
    There is a peculiar issue at the end of the year if the current year is a leap year.
    The issue leads to have a day less from the current timestamp.
    So DAY is initialized with one extra day when such a situation occurs */
    if( (Leapyearflag) && (total_t_years != (Year - 1970))){
        Day = 1;
    }
    else{
        Day = 0;
    }


    /* A not so efficient way to calculate the current Month and Day */
    for(int i=0; i<12; i++){
        if (days_left_in_year <= Month_Array[i]){
            Month = i + 1;
            Day += days_left_in_year % 31;
            break;
        }
        else{
            days_left_in_year = days_left_in_year - Month_Array[i];
        }
    }

    /* This adds the current day into the entire total.
    So if today is 6PM Jan 1st 1970. Technically there have been zero days that have gone by,
    and only 18 hours since time has begun, but still we don't say 18:00Hrs 0th Jan 1970. We add the 1 day,
    to indicate which day we're currently in so it becomes 18:00Hrs 1st Jan 1970. */
    Day += 1;


    /* Doomsday formula for calculating the Weekday of the Year.
    Weekday will have 1-7. Sun-1, Mon-2 ... Sat-7.*/
    Weekday = claculateWeekday(Year, Month, Day);

    seconds_in_day = temp_timestamp % 86400;

    minutes_in_day = (int) (seconds_in_day / 60);

    /* Hour will have 0-23 */
    Hour = (int) (minutes_in_day / 60);

    /* Minute will have 0-59 */
    Minute = minutes_in_day - (Hour * 60);
}

void rtcClass::updateTimeVariables(void){
    updateTimeVariables(now());
}


unsigned long rtcClass::year(unsigned long temptime){
    updateTimeVariables(temptime);
    return Year;
}

unsigned long rtcClass::year(void){
    updateTimeVariables(now());
    return Year;
}



unsigned long rtcClass::month(unsigned long temptime){
    updateTimeVariables(temptime);
    return Month;
}

unsigned long rtcClass::month(void){
    updateTimeVariables(now());
    return Month;
}



unsigned long rtcClass::day(unsigned long temptime){
    updateTimeVariables(temptime);
    return Day;
}

unsigned long rtcClass::day(void){
    updateTimeVariables(now());
    return Day;
}



unsigned long rtcClass::weekday(unsigned long temptime){
    updateTimeVariables(temptime);
    return Weekday;
}

unsigned long rtcClass::weekday(){
    updateTimeVariables(now());
    return Weekday;
}



unsigned long rtcClass::hour(unsigned long temptime){
    updateTimeVariables(temptime);
    return Hour;
}

unsigned long rtcClass::hour(void){
    updateTimeVariables(now());
    return Hour;
}



unsigned long rtcClass::minute(unsigned long temptime){
    updateTimeVariables(temptime);
    return Minute;
}

unsigned long rtcClass::minute(void){
    updateTimeVariables(now());
    return Minute;
}

