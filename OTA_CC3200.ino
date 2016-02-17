//=====================OTA update to CC3200 by FTP swerver===========================================
//Simple OTA update for CC3200 launchpad with FTP server on network PC
// use FTP server on windows PC that hold the programing file.
//example to FTP server on window PC:
// xlight.exe
//http://www.xlightftpd.com/download.htm
// define virtual server
//User Name : xxxxx
//PW: no need PW
//file upgrade Directory define in FTP server :  xxxxxxxx
//
// use SLFS-master last version in github
//tested on energia MT V17 and lunchpad CC3200
//TBD:  for safer program , need to add time out for the loops !!!!
//michanisani@gmail.com



// Do not include SPI for CC3200 LaunchPad
//#include <SPI.h>
#include <hw_types.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ccRTC.h>
#include <Wire.h>
#include <slfs.h>
#include "Energia.h"


char ssid[] = "xxx";      //  your network SSID (name)
char pass[] = "xxx";   // your network password

int status = WL_IDLE_STATUS;
int SERVER_PORT = 80;
IPAddress N_ip      (10,0,0,101);
IPAddress N_dns     (10,0,0,138);
IPAddress N_gateway (10,0,0,138);
IPAddress N_subnet  (255,0,0,0);
WiFiServer server(SERVER_PORT);
WiFiClient WIFI_client; // for server
WiFiClient FTP_client;
WiFiClient FTP_dclient;
//----------------- FTP client, write log file to server ---------------------------------------
// FTP change to your server
IPAddress FTP_server(10,0,0,1); // FTP PC server port 21 in PC IP=10.0.0.1
char FTP_outBuf[256]; // for controll
char FTP_outCount;
unsigned int FTP_hiPort,FTP_loPort;

//=============================================================================
void setup()
{
 Serial.begin(9600);      // initialize serial communication
//=================== WIFI ============================================
    WiFi.config(N_ip, N_dns, N_gateway, N_subnet);
    delay(4000);
    Serial.println("Attempting to connect to Network");
    WiFi.begin(ssid, pass);
    delay(4000);
    while ( WiFi.status() != WL_CONNECTED)
    {
   // print dots while we wait to connect
    Serial.print(".");
    delay(300);
    }
  Serial.println("Waiting for an ip address");
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }
  Serial.println("IP Address obtained");
  // you're connected now, so print out the status
  printWifiStatus();
//------------------------------- Start Serial Flash --------------------------------------------------
// BEGIN SLFS
 SerFlash.begin();//Essential fof SLFS sets up the DMA etc actually seems to do a WiFi.init(); function call
 Serial.println("CC3200 SLFS Serial Flash");
}

//=======================================================================================
void loop()
{
 if (Update_Firmware_By_FTP())  Serial.println("Flash File WRITTEN");
 else  Serial.println("Flash File write Fail");
 delay (5000);
 Serial.println("Rebot");
 RebootMCU(); // make soft reset here
// program do not get to here
 while(1);
}  // end main loop


//=======================================================================================
void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Adr: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//-------------------------------------------------------------------------------------------------------------
byte eRcv()
{
  byte respCode;
  byte thisByte;

  while(!FTP_client.available()) delay(1);

  respCode = FTP_client.peek();

  FTP_outCount = 0;

  while(FTP_client.available())
  {
    thisByte = FTP_client.read();
//    Serial.write(thisByte); // debug
    if(FTP_outCount < 127)
    {
      FTP_outBuf[FTP_outCount] = thisByte;
      FTP_outCount++;
      FTP_outBuf[FTP_outCount] = 0;
    }
  }
  if(respCode >= '4')
  {
    efail();
    return 0;
  }
  return 1;
}
//-------------------------------------------------------------------------------------------------------------
void efail()
{
  byte thisByte = 0;

  FTP_client.println("QUIT");

  while(!FTP_client.available()) delay(1);

  while(FTP_client.available())
  {
    thisByte = FTP_client.read();
//    Serial.write(thisByte); // debug
  }
  FTP_client.stop();
  Serial.println("Command disconnected");
  }

//-------------------------------------------------------------------------------------------------------------
//the SLFS library declares an instance right away called "SerFlash",
// SerFlash.open(), SerFlash.close(), SerFlash.write(), SerFlash.print(), SerFlash.println(), etc...
//You can delete files using SerFlash.del()
//You cannot list the directory contents or know what files are on the flash.  I did not find any API calls in the SimpleLink API that support such a feature.
//I'm assuming it's been left out for security reasons or similar.
//The open() system call is given options based on the sl_FsOpen() SimpleLink API call, so... looking at documentation here- http://software-dl.t...html/index.html ... look at "File System" and then "sl_FsOpen":
//The syntax for SerFlash.open() therefore is something like this:
//SerFlash.open("myfile.txt", FS_MODE_OPEN_READ);  // Open file for reading
//SerFlash.open("myfile.txt", FS_MODE_OPEN_CREATE(1024, _FS_FILE_OPEN_FLAG_COMMIT));  // Create new file, allocated 1024 bytes to contain it
//SerFlash.open("myfile.txt", FS_MODE_OPEN_WRITE);  // Open file for re-writing
//From what I've been able to gather, you can't open an existing file and write pieces of it piecemeal; opening the file and writing forces the serial flash to erase the entire block,
//so what you write replaces the prior contents entirely.  So for updating configuration info, you would need to read it into memory, make your changes and then re-write the entire file.
//Also, there's a maximum # of files you can store ... something like 128 I believe, and that includes a number of system files in that count which are already there.  It's not meant to be a huge filesystem.

char fileName[12] = "mcuimg.bin"; // Upgrade File name to to read from FTP server

byte Update_Firmware_By_FTP()
{
 int Readbyte;
 int32_t readStatus;
 int i,j;
 int endloop;
 long prog_file_size;

  readStatus= SerFlash.open("\\sys\\mcuimg.bin", FS_MODE_OPEN_READ);
  if (readStatus == SL_FS_OK)
  {
   Serial.println("--Found Flash file = ");
   SerFlash.close();// close the file
  }
 else
 {
    Serial.print("Error opening \\sys\\mcuimg.bin error code: ");
    Serial.println(SerFlash.lastErrorString());
    SerFlash.close();// close the file
    return (0);
 }
  // delete image file , we will write new one
  readStatus=SerFlash.del("\\sys\\mcuimg.bin");
  Serial.print("Deleting \\sys\\mcuimg.bin return code: ");
  Serial.println(SerFlash.lastErrorString());

  // CREATES THE FILE ---------------------------
  readStatus=SerFlash.open("\\sys\\mcuimg.bin", FS_MODE_OPEN_CREATE(128*1024, _FS_FILE_OPEN_FLAG_COMMIT));// create and open the file with this name, max size 128K
  if (readStatus != SL_FS_OK)
    {
    Serial.print("Error creating file \\sys\\mcuimg.bin, error code: ");
    Serial.println(SerFlash.lastErrorString());
    SerFlash.close();// close the file
    return (0);
  }
  else
  {
    Serial.print("File created: \\sys\\mcuimg.bin");
    SerFlash.close();// close the file
  }

  // THIS OPENS THE FILE FOR WRITING -----------------
  // Great!  Now store the contents into the same file, re-creating it (thus overwriting).
  readStatus=SerFlash.open("\\sys\\mcuimg.bin", FS_MODE_OPEN_WRITE);  //open file for writing to
  if (readStatus != SL_FS_OK)
    {
    Serial.print("Err file \\sys\\mcuimg.bin for write,Err-");
    Serial.println(SerFlash.lastErrorString());
    SerFlash.close();// close the file
    return (0);
  }
  Serial.print("Flash File Opened for wr: ");
  Serial.println(SerFlash.lastErrorString());

//--------------------------- do FTP read and write to Flash ------------------------------------------
  if (FTP_client.connect(FTP_server,21)) {   // Server port 21
    Serial.println("FTP connect");
  }
  else
   {
      Serial.println("FTP conn failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
   }

  if(!eRcv())
    {
      Serial.println("FTP conn failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
    }

  FTP_client.println("USER boiler");  //<====
  if(!eRcv())
    {
      Serial.println("User failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
    }

  FTP_client.println("PASS"); //<===
  if(!eRcv())
    {
      Serial.println("PW failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
    }

  //Syntax: SYST
  //Returns a word identifying the system, the word "Type:", and the default transfer type (as would be set by the TYPE command). For example: UNIX Type: L8
  FTP_client.println("SYST"); //<==== Return system type
  if(!eRcv())
    {
      Serial.println("SYST failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
    }
  FTP_client.println("Type I"); //<==== Sets the transfer mode (ASCII/Binary).  A - ASCII text  E - EBCDIC text I - image (binary data) L - local format
  if(!eRcv())
    {
      Serial.println("TypI failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
    }

//Syntax: PASV
//Tells the server to enter "passive mode". In passive mode, the server will wait for the client to establish a connection with it rather than attempting to connect to a client-specified port.
//The server will respond with the address of the port it is listening on, with a message like:
//227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)
//where a1.a2.a3.a4 is the IP address and p1*256+p2 is the port number.
  FTP_client.println("PASV"); // <==== Enter passive mode
  if(!eRcv())
    {
      Serial.println("Pasv failed");
      SerFlash.close();// close the file
      FTP_client.stop();
      return 0;
    }

  char *tStr = strtok(FTP_outBuf,"(,");
  int array_pasv[6];
  for ( int i = 0; i < 6; i++)
 {
    tStr = strtok(NULL,"(,");
    array_pasv[i] = atoi(tStr);
    if(tStr == NULL)
    {
      Serial.println("Bad PASV Answer");
      SerFlash.close();// close the file
      FTP_dclient.stop();
      FTP_client.stop();
      return 0;
    }
  }

  unsigned int hiPort,loPort;

  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  Serial.print("Data port: ");
  hiPort = hiPort | loPort;
  Serial.println(hiPort);

  if (FTP_dclient.connect(FTP_server,hiPort)) {
    Serial.println("Data connected");
  }
  else
   {
    Serial.println("Data conn failed");
    SerFlash.close();// close the file
    FTP_dclient.stop();
    FTP_client.stop();
    return 0;
   }
//------- get file size -----------------
 FTP_client.println("SIZE mcuimg.bin");
  if(!eRcv())
  {
    Serial.println("Size failed");
    SerFlash.close();// close the file
    FTP_dclient.stop();
    FTP_client.stop();
    return 0;
  }
  // calculate file size
  char *pch;
  pch=strchr(FTP_outBuf,' ');
  if (pch == NULL)
  {
    Serial.println("Size do not have leading space");
    SerFlash.close();// close the file
    FTP_dclient.stop();
    FTP_client.stop();
    return 0;
  }
  prog_file_size = 0;
  sscanf(pch, "%ld", &prog_file_size);
  Serial.print("read file size=");
  Serial.println(prog_file_size);
  if ((prog_file_size <= 0) || (prog_file_size > (128 * 1024))) // 128K CPU
  {
    Serial.println("server file size too small/big");
    SerFlash.close();// close the file
    FTP_dclient.stop();
    FTP_client.stop();
    return 0;
  }

//------------------- read the file
  FTP_client.print("RETR "); //<==== Retrieve a copy of the file
  FTP_client.println(fileName); // name of upgrade file

  if(!eRcv())
  {
      Serial.println("RETR failed");
      SerFlash.close();// close the file
      FTP_dclient.stop();
      FTP_client.stop();
      return 0;
  }

  endloop=1;

  while(FTP_dclient.connected() && endloop )
  {
    while(FTP_dclient.available() && endloop )
    {
      char c = FTP_dclient.read();
      if ((readStatus = SerFlash.write(c))==0)  // write content to the file
      {
        Serial.print("Error flash Wr-");
        Serial.println(SerFlash.lastErrorString());
        SerFlash.close();// close the file
        FTP_dclient.stop();
        FTP_client.stop();
        return 0;
      }
       prog_file_size--;
       if((prog_file_size % (1024*5))==0) Serial.println(prog_file_size); // progress indication every 5K
       if (prog_file_size <= 0)
         {
          endloop=0; // end
          Serial.println("End Read");
         }
    }
  }

  FTP_dclient.stop();
  Serial.println("Quit FTP");
  FTP_client.println("QUIT");
  if(!eRcv())
    {
      Serial.println("Quit failed");
      SerFlash.close();// close the file
      FTP_dclient.stop();
      FTP_client.stop();
      return 0;
    }

  FTP_client.stop();
  Serial.println("Command disconnected");
  SerFlash.close();// close the file
  return 1;
}


//==================================================================================================
//****************************************************************************
//
//! Reboot the MCU by requesting hibernate for a short duration
//!
//! \return None
//
//****************************************************************************
void RebootMCU()
{

  //
  // Configure hibernate RTC wakeup
  //
  PRCMHibernateWakeupSourceEnable(PRCM_HIB_SLOW_CLK_CTR);
  //
  // Delay loop
  //
 // MAP_UtilsDelay(8000000);
  delay(1000);
  //
  // Set wake up time
  //
  PRCMHibernateIntervalSet(330);
  //
  // Request hibernate
  //
  PRCMHibernateEnter();
  //
  // Control should never reach here
  //
  while(1)
  {

  }
}


//===================== Open Bugs =========================




































