#include <Energia.h>
#include <hw_types.h>
#include <driverlib/prcm.h>
//#include <d:\ti\energia-0101E0016\hardware\cc3200emt\cores\cc3200emt\driverlib\prcm.h>


class rtcClass
{
    private:
        boolean Beginstatus;
        boolean Leapyearflag;
        float Timezone;
        int Timezoneadjust;
        int Minute;
        int Hour;
        int Day;
        int Weekday;
        int Month;
        int Year;
        int Month_Array[12];
        int Weekday_Lookup[12];

        int claculateWeekday(int y, int m, int d);

        void updateTimeVariables(unsigned long Timestamp);
        void updateTimeVariables(void);



    public:
        rtcClass();

        /* set PRCM RTC Flag and start the RTC time from 0 seconds i.e. 00:00Hrs Jan 1st 1970 */
        void begin(void);

        /* set timezone as float value -12 to +12 */
        void zone(float timezone);

        /* set RTC time as the number of seconds since 00:00Hrs Jan 1st 1970 */
        void setTime(unsigned long newtime);

        /* returns number of seconds since 00:00Hrs Jan 1st 1970 */
        unsigned long now(void);

        /* returns YEAR in the YYYY format. Starts from 1970 */
        unsigned long year(unsigned long temptime);
        unsigned long year(void);

        /* returns 1-12 */
        unsigned long month(unsigned long temptime);
        unsigned long month(void);

        /* returns 1-31 */
        unsigned long day(unsigned long temptime);
        unsigned long day(void);

        /* returns 1-7. Sun-1, Mon-2 ... Sat-7 */
        unsigned long weekday(unsigned long temptime);
        unsigned long weekday();

        /* returns 0-23 */
        unsigned long hour(unsigned long temptime);
        unsigned long hour(void);

        /* returns 0-59 */
        unsigned long minute(unsigned long temptime);
        unsigned long minute(void);
};

extern rtcClass ccRTC;

