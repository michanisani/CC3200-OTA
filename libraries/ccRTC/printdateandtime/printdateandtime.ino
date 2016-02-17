#include <ccRTC.h>

char weekdayString[4] = "Sun";

void weekdayToString(int weekdaycode){
 switch (weekdaycode){
   case 1:
     memcpy(weekdayString, "Sun", 3);
     break;
   case 2:
     memcpy(weekdayString, "Mon", 3);
     break;
   case 3:
     memcpy(weekdayString, "Tue", 3);
     break;
   case 4:
     memcpy(weekdayString, "Wed", 3);
     break;
   case 5:
     memcpy(weekdayString, "Thu", 3);
     break;
   case 6:
     memcpy(weekdayString, "Fri", 3);
     break;
   case 7:
     memcpy(weekdayString, "Sat", 3);
     break;
 }
}

void setup()
{
  Serial.begin(9600);
  ccRTC.begin();
  ccRTC.setTime(1388534400);  // init to 1st Jan 2014, 12:00AM
}

void loop()
{

  Serial.print(ccRTC.day());
  Serial.print("-");
  Serial.print(ccRTC.month());
  Serial.print("-");
  Serial.print(ccRTC.year());
  Serial.print(" ");
  Serial.print(ccRTC.hour());
  Serial.print(":");
  Serial.print(ccRTC.minute());
  Serial.print(" ");
  weekdayToString(ccRTC.weekday());
  Serial.println(weekdayString);

  delay(1000);
}

