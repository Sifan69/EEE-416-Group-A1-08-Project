//Source Code for Bangla Calendar Clock 3.0 for EEE 416 Lab Project, BUET - Dhaka, Bangladesh
//Credits to Lab Partners: Rifat Ullah, Khandakar Humyra Oyshi, Sabir Mahmud, Nafis Faisal

#include "RTClib.h"
#include <SPI.h>        
#include "SystemFont5x7.h"
#include <EEPROM.h>
#include <DMD2.h>
#define DISPLAYS_ACROSS 2 //-> Number of P10 panels used, side to side.
#define DISPLAYS_DOWN 1

RTC_DS1307 rtc; //-> RTC Declaration
SoftDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);  
            
#define serialCommunicationSpeed 115200               
#define time_out 1000
#define DEBUG 1

uint8_t en_m_size[13] = {0, 43, 49, 19, 31, 13, 19, 30, 36, 47, 44, 38, 42};
uint8_t bn_m_size[13] = {0, 33, 23, 34, 28, 23, 35, 38, 48, 25, 20, 33, 22};
uint8_t wk_size[8]  = {0, 23, 21, 27, 30, 18, 24, 27};
uint8_t d_size = 19; //8+1+8+2
uint8_t y_size = 37; //2 + (4*8 + 3)

//uint8_t M1 = 12;  //sizeof(disp1) / sizeof(disp1[0]);
uint8_t N1 = 8; //sizeof(disp1[0]) / sizeof(disp1[0][0]);
uint32_t lo_op=0; //selects which column from the disp matrix will appear first
uint8_t second_loop = 0;

//time variables 

uint8_t sync_time[6];

uint8_t tim_e[6];
uint8_t ampm=0; //0 if AM,1 if PM

uint8_t en_date[2];
uint8_t en_year[4];
uint8_t en_m; //month

uint8_t bn_date[2];
uint8_t bn_year[4];
uint8_t bn_m; //month

uint8_t week; 

uint8_t count = 1;
uint8_t scroll = 1;

char sc = '"';
String rec_command;
String rec_data;
//String response = "";
uint8_t prev;

uint8_t dmd_flag = 0; // 0 = DMD is OFF, 1 = DMD is off

long int elapsed_time = 0;
uint8_t prev_min = 0;
uint8_t prev_sec = 0;
uint8_t trans_flag = 1;
uint8_t wifi_timeout = 0;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//Main code variables

int _day, _month, _year, _hour24, _hour12, _minute, _second, _dtw;
int hr24;
String st;
char am_pm[3];
char nameoftheday[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char month_name[12][12] = {"January","February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
char bangla_month_name[12][12] = {"Baishakh", "Jaishtha", "Ashar", "Shrabon", "Bhadra", "Ashwin", "Kartik", "Agrahayon", "Poush", "Magh", "Falgun", "Chaitra"};

const long interval = 1000; //-> Retrieve time and date data every 1 second
unsigned long previousMillis = 0;





void setup() {
  Serial.begin(serialCommunicationSpeed);
  dmd.setBrightness(255);
  
  load_rtc();
  delay(100);

  Serial.println("Arduino RTC DS3231");
  delay(1000);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled (Set the time and date based on your computer time and date)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //-> Set the time and date based on your computer time and date. If that doesn't work, use this line of code outside of "if (! rtc.isrunning())"
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  // following line sets the RTC to the date & time this sketch was compiled (Set the time and date based on your computer time and date)
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //-> Set the time and date based on your computer time and date. Use this line of code if it doesn't work in "if (! rtc.isrunning())"
  //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  // If the time and date are successfully set, then deactivate the code line (make the code a comment), then re-upload the code.
  // if not done then the time and date will return to the beginning when it was set when arduino is reset or restarted.

  //Timer1.initialize(1000);          
  dmd.scanDisplay();  
  dmd.clearScreen();
  dmd.begin();  
  dmd.selectFont(SystemFont5x7);
}

void loop()                                                         
{
  //unsigned long currentMillis = millis();
  //if (currentMillis - previousMillis >= interval) {
    //previousMillis = currentMillis; //-> save the last time

  //GetDateTime(); //-> Retrieve time and date data from DS3231
  //Serial.println("Loading and Showing RTC Time...");

  //Serial.end();
  load_rtc();

  //Data at the time of the sync
  //prev_min = myRTC.minutes; //rtc time                            //these two have been commented out
  //prev_sec = myRTC.seconds; //rtc time
  elapsed_time = millis(); //arduino internal time
  ///////
  //dmd.begin();
  dmd_flag = 1;
  //populate_cells();

  long int current_time = millis();
  if(((abs(elapsed_time - current_time)) >= (interval)))
  {
    elapsed_time = current_time; 
    //dmd.end();
    load_rtc();
    if((prev_min != _minute) && (trans_flag == 1))
    {
      elapsed_time = current_time;
      trans_flag = 0;
    }

    prev_min = _minute;
    prev_sec = _second;
    //dmd.begin();
  }

  populate_cells(); 

  //load_rtc();
    
}
////// void loop ends ///////

void load_rtc(void)
{
  GetDateTime();
  //Serial.println(myRTC.hours);
  if(_hour24 >= 13)
  {
    tim_e[0] = (_hour24-12)/10;
    tim_e[1] = (_hour24-12)%10;
  }
  else if(_hour24 == 0)
  {
    tim_e[0] = 1;
    tim_e[1] = 2;
  }
  else
  {
    tim_e[0] = (_hour24)/10;
    tim_e[1] = (_hour24)%10;
  }

  tim_e[2] = (_minute)/10;
  tim_e[3] = (_minute)%10;

  tim_e[4] = _second/10;
  tim_e[5] = _second%10;

  if(_hour24 >= 12)
    ampm = 1; //0 if AM,1 if PM
  else
    ampm = 0; //0 if AM,1 if PM

  week = _dtw;

  en_date[0] = _day/10;
  en_date[1] = _day%10;

  en_m = _month;
  
  en_year[0] = _year/1000;
  en_year[1] = (_year%1000)/100;
  en_year[2] = ((_year%1000)%100)/10;
  en_year[3] = ((_year%1000)%100)%10;

  set_ban_date();

}

void GetDateTime() {
  DateTime now = rtc.now();
  _day=now.day();
  _month=now.month();
  _year=now.year();
  _hour24=now.hour();
  _minute=now.minute();
  _second=now.second();
  _dtw=now.dayOfTheWeek();
  if (_dtw == 6) _dtw = 1;
  else _dtw += 2;
  //en_m = _month;  
}

/*---------------------------------------------------------------------------------------------------------------------------
 *---------------------------------------INFO ON BANGLA MONTHS---------------------------------------------------------------
 * They can be easily calculated from the english months (leap year taken into account)
 * Of 365 days, the first 6 bangla months are 31 days// Falgun (february time) is 30 during leap year, else 29// Rest are 30
 * For reference and more data, See: https://en.wikipedia.org/wiki/Bengali_calendars
 * The leap year adjusted bangla date have been standardized in 2019 
 * WARNING: Indian Bangla Date (West Bengal) is different from that of the country "Bangladesh"
 * Here we are following the Bangla Date followed and observed by the people of Bangladesh
 * --------------------------------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------------------------------
 */
 
void set_ban_date(void)
{
      int leap_year = 1;
      
      //load_rtc();
      
      int en_yea_r = _year;
      int en_dat_e = _day;

      //check for leap year
      //if( ((en_yea_r%400 == 0) && (en_yea_r%100 != 0)) || ((en_yea_r%4 == 0) && (en_yea_r%100 != 0)) )
      //  leap_year = 1;
      //else
      //  leap_year = 0;

      int yea_r;
      int dat_e = en_dat_e;

      if(en_m>=4 && en_m<=12) //April to December
      {
        if((en_m==4) && (en_dat_e <= 13)) //Before Pahela Baishakh
          yea_r=en_yea_r-593-1;
        else
          yea_r=en_yea_r-593;
      }
      else
      {
        yea_r=en_yea_r-593-1; //January to March
      }
      
      bn_year[0] = yea_r/1000;
      bn_year[1] = (yea_r%1000)/100;
      bn_year[2] = (yea_r%100)/10;
      bn_year[3] = ((yea_r%1000)%100)%10;
      
      if(en_m==4) //April 13 = Chaitra 30 -- April 14 = Boishakh 1 //See More: https://en.wikipedia.org/wiki/Pohela_Boishakh
      {

        if(en_dat_e<=13)
        {
           bn_m=12;
           dat_e=dat_e+17;
        }
        else if(en_dat_e>13)
        {
           en_m=1;
           dat_e=dat_e-13;
        }
      }
      else if(en_m==5) //May 14 = Boisakh 31 -- May 15 = Jaistha 1
      {
        if(en_dat_e<=14)
        {
           bn_m=1;
           dat_e=dat_e+17;
        }
        else if(en_dat_e>14)
        {
           bn_m=2;
           dat_e=dat_e-14;
        }
      }
    else if(en_m==6) //June 14 = Jaistha 31 -- June 15 = Asharh 1
    {
      if(en_dat_e<=14)
      {
        bn_m=2;
        dat_e=dat_e+17;
      }
      else if(en_dat_e>14)
      {
        bn_m=3;
        dat_e=dat_e-14;
      }
    }
    else if(en_m==7) //July 15 = Asharh 31 -- July 16 = Sraban 1
    {
      if(en_dat_e<=15)
      {
        bn_m=3;
        dat_e=dat_e+16;
      }
      else if(en_dat_e>15)
      {
        bn_m=4;
        dat_e=dat_e-15;
      }
    }
    else if(en_m==8) //August 15 = Sraban 31 -- August 16 = Vhadra 1
    {
      if(en_dat_e<=15)
      {
        bn_m=4;
        dat_e=dat_e+16;
      }
      else if(en_dat_e>15)
      {
        bn_m=5;
        dat_e=dat_e-15;
        }
    }
    else if(en_m==9) //September 15 = Vhadra 31 -- September 16 = Ashwin 1
    {
      if(en_dat_e<=15)
        {
          bn_m=5;
          dat_e=dat_e+16;
        }
        else if(en_dat_e>15)
        {
           bn_m=6;
           dat_e=dat_e-15;
        }
    }
    else if(en_m==10) //October 16 = Ashwin 31 -- October 17 = Kartik 1
    {
        if(en_dat_e<=16)
        {
           bn_m=6;
           dat_e=dat_e+15;
        }
        else if(en_dat_e>16)
        {
            bn_m=7;
            dat_e=dat_e-16;
        }
    }
    else if(en_m==11) //November 15 = Kartik 30 -- November 16 = Ogrohayon 1
    {
        if(en_dat_e<=15)
        {
          bn_m=7;
          dat_e=dat_e+15;
        }
        else if(en_dat_e>15)
        {
           bn_m=8;
           dat_e=dat_e-15;
        }
    }
    else if(en_m==12) //December 15 = Ogrohayon 30 -- December 16 = Poush 1
    {
        if(en_dat_e<=15)
        {
          bn_m=8;
          dat_e=dat_e+15; 
        }
        else if(en_dat_e>15)
        {
           bn_m=9;
           dat_e=dat_e-15;
        }
    }
    else if(en_m==1) //January 14 = Poush 30 -- January 15 = Magh 1
    {
        if(en_dat_e<=14)
        {
           bn_m=9;
           dat_e=dat_e+16;
        }
        else if(en_dat_e>14)
        {
           bn_m=10;
           dat_e=dat_e-14;
        }

    }
    else if(en_m==2) //February 13 = Magh 30 -- February 14 = Falgun 1 //See More: https://en.wikipedia.org/wiki/Pahela_Falgun
    {
        if(en_dat_e<=13)
        {
           bn_m=10;
           dat_e=dat_e+17;
        }
        else if(en_dat_e>13)
        {
           bn_m=11;
           dat_e=dat_e-13;
        }
    }
    //Falgun - February upto Baisakh - April are the months subject to leap year adjustment //In short, only this last "if" statement is logic adjusted
    else if(en_m==3) 
    {
      if(leap_year == 0) //NOT A LEAP YEAR: March 14 = Falgun 29 -- March 15 = Chaitra 1 (February 28 = Falgun 15)
      {
        if(en_dat_e<=14)
        {
           bn_m=11;
           dat_e=dat_e+15;
        }
        else if(en_dat_e>14)
        {
           bn_m=12;
           dat_e=dat_e-14;
        }
      }
      if(leap_year == 0) //LEAP YEAR: March 14 = Falgun 30 -- March 15 = Chaitra 1 (February 29 = Falgun 16)
      {
        if(en_dat_e<=14)
        {
           bn_m=11;
           dat_e=dat_e+16;
        }
        else if(en_dat_e>14)
        {
           bn_m=12;
           dat_e=dat_e-14;
        }
      }
    }
    
    bn_date[0] = dat_e/10;
    bn_date[1] = dat_e%10;
    //Serial.println(en_m);
    //Serial.println(dat_e);
    //Serial.println(yea_r);
      //rec_data = "";
      //count = count+1;
}

///////////////////////////////// MATRIX MANIPULATION /////////////////////////////////////////////////////////////////
//matrices for English months
uint64_t en_months( uint8_t en_m, uint8_t i, uint8_t shft ){
  
 uint64_t temp = 0;

 if (en_m==1){
    // JANUARY, size = 43
    if      (i==0)  temp = 0b0000000000000000000000000000000000011111100; //New
    else if (i==1)  temp = 0b0000000000000000000000000000000000100000011; //New
    else if (i==2)  temp = 0b0000000000000000000000000000000000100000000; //New
    else if (i==3)  temp = 0b1111111111110111111111111111111011111111111; //New
    else if (i==4)  temp = 0b0000101100001000000010011000100100100000010; //New
    else if (i==5)  temp = 0b1001000011101000000010000100100100100001110; //New
    else if (i==6)  temp = 0b1001001000101001111010001110100100100110010; //New
    else if (i==7)  temp = 0b1001001101001001101010010000100100100100010; //New
    else if (i==8)  temp = 0b0100110101001001110110011100100100100011010; //New
    else if (i==9)  temp = 0b0100000101001000000110000010100100100000110; //New
    else if(i==10)  temp = 0b0010001001001000000010000001100100100010010; //New
    else if(i==11)  temp = 0b0001111001001000000010000100100100100010010; //New
    else if(i==12)  temp = 0b0000000000100000001111000100000000000000000; //New
    else if(i==13)  temp = 0b0000000000000000001011100000000000000000000; //New
    else if(i==14)  temp = 0b0000000000000000001110010000000000000000000; //New
    else if(i==15)  temp = 0b0000000000000000000000000000000000000000000; //New
 }

 else if (en_m==2){
    // FEBRUARY, size = 49
    if      (i==0)  temp = 0b0000000000000000000000000000000000000000011111100; //New
    else if (i==1)  temp = 0b0000000000000000000000000000000000000000100000011; //New
    else if (i==2)  temp = 0b0000000000000000000000000000000000000000100000000; //New
    else if (i==3)  temp = 0b0111111111111111111111111111111111111011111111111; //New
    else if (i==4)  temp = 0b0100011000000000000000100000110000100100100000010; //New
    else if (i==5)  temp = 0b1000001100011110000011101100001000100100100000010; //New
    else if (i==6)  temp = 0b1000001100010001001110101110011100100100100001110; //New
    else if (i==7)  temp = 0b1000010000010101001000100010100000100100100110010; //New
    else if (i==8)  temp = 0b1000011100010110000111111100111000100100100100010; //New
    else if (i==9)  temp = 0b1000000010010000000001100000000100100100100011010; //New
    else if(i==10)  temp = 0b1000000001110000010000100000000010100100100000110; //New
    else if(i==11)  temp = 0b0111000000110000010011100000001001100100100010010; //New
    else if(i==12)  temp = 0b0111000000010000011100100000001000100100100010010; //New
    else if(i==13)  temp = 0b0000000000000000000000000000000000000000000000000; //New
    else if(i==14)  temp = 0b0000000000000000000000000000000000000000000000000; //New
    else if(i==15)  temp = 0b0000000000000000000000000000000000000000000000000; //New
 }

 else if (en_m==3){
  // MARCH, size = 19
    if      (i==0)  temp = 0b0000000000000000100; //New
    else if (i==1)  temp = 0b0000000000000011000; //New
    else if (i==2)  temp = 0b0000000000000110000; //New
    else if (i==3)  temp = 0b1111111111011111111; //New
    else if (i==4)  temp = 0b0110000100100100000; //New
    else if (i==5)  temp = 0b0001000100100110000; //New
    else if (i==6)  temp = 0b0000100100100101110; //New
    else if (i==7)  temp = 0b0000100100100100010; //New
    else if (i==8)  temp = 0b0011100100100100010; //New
    else if (i==9)  temp = 0b0011111100100100100; //New
    else if(i==10)  temp = 0b0000001100100100100; //New
    else if(i==11)  temp = 0b0000000100100011000; //New
    else if(i==12)  temp = 0b0000000000000000000; //New
    else if(i==13)  temp = 0b0000000000000000000; //New
    else if(i==14)  temp = 0b0000000000000000000; //New
    else if(i==15)  temp = 0b0000000000000000000; //New
 }

 else if (en_m==4){
  // APRIL, size = 31
    if      (i==0)  temp = 0b0000000000011111100000000000000; //New
    else if (i==1)  temp = 0b0000000000100000011000000000000; //New
    else if (i==2)  temp = 0b0000000000100000000000000000000; //New
    else if (i==3)  temp = 0b0000011101110001110011111111111; //New
    else if (i==4)  temp = 0b0000110100100010001010000000010; //New
    else if (i==5)  temp = 0b0001000100100011101110011111110; //New
    else if (i==6)  temp = 0b0001000100100000110110100010010; //New
    else if (i==7)  temp = 0b0001110100100000100010101000010; //New
    else if (i==8)  temp = 0b0000110100100000000010111000010; //New
    else if (i==9)  temp = 0b0000000100100000000010011000010; //New
    else if(i==10)  temp = 0b1000000100100100000010000000010; //New
    else if(i==11)  temp = 0b1000111100100100011110000000010; //New
    else if(i==12)  temp = 0b0111000100100011100010000000010; //New
    else if(i==13)  temp = 0b0000000000000000000000000000000; //New
    else if(i==14)  temp = 0b0000000000000000000000000000000; //New
    else if(i==15)  temp = 0b0000000000000000000000000000000; //New
 }

 else if (en_m==5){
  // MAY, size = 13
    if      (i==0)  temp = 0b0000000000000; //New
    else if (i==1)  temp = 0b0000000000000; //New
    else if (i==2)  temp = 0b0000000000000; //New
    else if (i==3)  temp = 0b0111111111111; //New
    else if (i==4)  temp = 0b0100011000010; //New
    else if (i==5)  temp = 0b1000000100010; //New
    else if (i==6)  temp = 0b1000000010010; //New
    else if (i==7)  temp = 0b1000000010010; //New
    else if (i==8)  temp = 0b1000001110010; //New
    else if (i==9)  temp = 0b1000001111110; //New
    else if(i==10)  temp = 0b1111000000110; //New
    else if(i==11)  temp = 0b0111000000010; //New
    else if(i==12)  temp = 0b0000000000000; //New
    else if(i==13)  temp = 0b0000000000000; //New
    else if(i==14)  temp = 0b0000000000000; //New
    else if(i==15)  temp = 0b0000000000000; //New
 }

 else if (en_m==6){
  // JUNE, size = 19
    if      (i==0)  temp = 0b0000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000;
    else if (i==2)  temp = 0b1111111111111111111;
    else if (i==3)  temp = 0b0000101100000000010;
    else if (i==4)  temp = 0b1001000011100000010;
    else if (i==5)  temp = 0b1001001000101111010;
    else if (i==6)  temp = 0b1001001101001101010;
    else if (i==7)  temp = 0b0100110101001110110;
    else if (i==8)  temp = 0b0100000101000000110;
    else if (i==9)  temp = 0b0010001001000000010;
    else if(i==10)  temp = 0b0001111001000000010;
    else if(i==11)  temp = 0b0000000011110000000;
    else if(i==12)  temp = 0b0000000010111000000;
    else if(i==13)  temp = 0b0000000011100100000;
    else if(i==14)  temp = 0b0000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000;
 }

 else if (en_m==7){ 
  //JULY, size = 30
    if      (i==0)  temp = 0b000000000000000000000010000000;
    else if (i==1)  temp = 0b000000000000000000000001111100;
    else if (i==2)  temp = 0b000000000000000000000000000010;
    else if (i==3)  temp = 0b111111111111111111111111111111;
    else if (i==4)  temp = 0b000010110000000000010100010000;
    else if (i==5)  temp = 0b100100001110011111110100011100;
    else if (i==6)  temp = 0b100100100010100010010100110010;
    else if (i==7)  temp = 0b100100110100100000010100000010;
    else if (i==8)  temp = 0b010011010100111000010100000010;
    else if (i==9)  temp = 0b010000010100011000010100111100;
    else if(i==10)  temp = 0b001000100100000000010100001100;
    else if(i==11)  temp = 0b000111100100000000010100000010;
    else if(i==12)  temp = 0b000000001111000000010100000001;
    else if(i==13)  temp = 0b000000001011100000000000000000;
    else if(i==14)  temp = 0b000000001110010000000000000000;
    else if(i==15)  temp = 0b000000000000000000000000000000;
 }

 else if (en_m==8){
  // AUGUST, size = 36
    if      (i==0)  temp = 0b000000000000000000000000000001000000; //New
    else if (i==1)  temp = 0b000000000000000000000000000001111110; //New
    else if (i==2)  temp = 0b000000000000000000000000000000000001; //New
    else if (i==3)  temp = 0b111111111111101001110011111111111111; //New
    else if (i==4)  temp = 0b000010000010010010001110011000100000; //New
    else if (i==5)  temp = 0b010001110010010011100110000100100000; //New
    else if (i==6)  temp = 0b010011111010010000010010000111100110; //New
    else if (i==7)  temp = 0b010001001010010000010010010100100101; //New
    else if (i==8)  temp = 0b010000001010010000100010011000100001; //New
    else if (i==9)  temp = 0b001000001010010001100010000000100001; //New
    else if(i==10)  temp = 0b000100011010010000000010000000110010; //New
    else if(i==11)  temp = 0b000011110110010000000010000000011100; //New
    else if(i==12)  temp = 0b000000000010010000000010000000000000; //New
    else if(i==13)  temp = 0b000000000010000000000000000000000000; //New
    else if(i==14)  temp = 0b000000000000000000000000000000000000; //New
    else if(i==15)  temp = 0b000000000000000000000000000000000000; //New
 }

 else if (en_m==9){
  // SEPTEMBER, size = 47
    if      (i==0)  temp = 0b00000000000000000000000001000000000000000000000; //New
    else if (i==1)  temp = 0b00000000000000000000000001111110000000000000000; //New
    else if (i==2)  temp = 0b00000000000000000000000000000001000000000000000; //New
    else if (i==3)  temp = 0b01111111111111111100111100111111111111111111111; //New
    else if (i==4)  temp = 0b01000110000100010001000010100000001000010000010; //New
    else if (i==5)  temp = 0b10000001000100100001110111100000011100010001110; //New
    else if (i==6)  temp = 0b10000000111100100000011001101100011011010110010; //New
    else if (i==7)  temp = 0b10000000100100100000010000101110000011100100010; //New
    else if (i==8)  temp = 0b10000101000100100000000000100010001100100011010; //New
    else if (i==9)  temp = 0b10000011000100100000000000100010001100100000110; //New
    else if(i==10)  temp = 0b11110000000100111100000000100100000011100010010; //New
    else if(i==11)  temp = 0b01110000000100011100000000011000000001100010010; //New
    else if(i==12)  temp = 0b00000000000000000000000000000000000000100000000; //New
    else if(i==13)  temp = 0b00000000000000000000000000000000000000000000000; //New
    else if(i==14)  temp = 0b00000000000000000000000000000000000000000000000; //New
    else if(i==15)  temp = 0b00000000000000000000000000000000000000000000000; //New
 }

 else if (en_m==10){
 // OCTOBER, Size 44
    if      (i==0)  temp = 0b00000000000000000001000000000000000000000000; //New
    else if (i==1)  temp = 0b00000000000000000000111111000000000000000000; //New
    else if (i==2)  temp = 0b00000000000000000000000001000000000000000000; //New
    else if (i==3)  temp = 0b11111111111111111111111111110111111111111111; //New
    else if (i==4)  temp = 0b00001000001000100000001000001000000100000010; //New
    else if (i==5)  temp = 0b01000111001001000000111110001000011100001110; //New
    else if (i==6)  temp = 0b01001111101001000011101011001001100100110010; //New
    else if (i==7)  temp = 0b01000100101001000010001111001001000100100010; //New
    else if (i==8)  temp = 0b00100000101001000001101111001000110100011010; //New
    else if (i==9)  temp = 0b00010001101001000000011000001000001100000110; //New
    else if(i==10)  temp = 0b00001111011001111000001011001000000100010010; //New
    else if(i==11)  temp = 0b00000000001000111000001111001000000100010010; //New
    else if(i==12)  temp = 0b00000000000000000000001011000000000000000000; //New
    else if(i==13)  temp = 0b00000000000000000000000110000000000000000000; //New
    else if(i==14)  temp = 0b00000000000000000000000000000000000000000000; //New
    else if(i==15)  temp = 0b00000000000000000000000000000000000000000000; //New
 }

 else if (en_m==11){
 // NOVEMBER, size = 38
    if      (i==0)  temp = 0b00000000000000000000000000000000000000; //New
    else if (i==1)  temp = 0b00000000000000000000000000000000000000; //New
    else if (i==2)  temp = 0b00000000000000000000000000000000000000; //New
    else if (i==3)  temp = 0b11111111111111111111111111111111111111; //New
    else if (i==4)  temp = 0b00000010001000000000000001000010000010; //New
    else if (i==5)  temp = 0b00000010010000000100000011100010001110; //New
    else if (i==6)  temp = 0b01111010010000101110100011011010110010; //New
    else if (i==7)  temp = 0b01101010010000101111010000011100100010; //New
    else if (i==8)  temp = 0b01110110010000100000010001100100011010; //New
    else if (i==9)  temp = 0b00000110010000010000010001100100000110; //New
    else if(i==10)  temp = 0b00000010011110001000100000011100010010; //New
    else if(i==11)  temp = 0b00000010001110000111000000001100010010; //New
    else if(i==12)  temp = 0b00000000000000000000000000000100000000; //New
    else if(i==13)  temp = 0b00000000000000000000000000000000000000; //New
    else if(i==14)  temp = 0b00000000000000000000000000000000000000; //New
    else if(i==15)  temp = 0b00000000000000000000000000000000000000; //New
 }

 else if (en_m==12){
 // DECEMBER, size = 42
    if      (i==0)  temp = 0b001111110000000000000000000000000000000000; //New
    else if (i==1)  temp = 0b010000001100000000000000000000000000000000; //New
    else if (i==2)  temp = 0b010000000000000000000000000000000000000000; //New
    else if (i==3)  temp = 0b111111111111111111111111111111111111111111; //New
    else if (i==4)  temp = 0b010000010000000100011000010001000010000010; //New
    else if (i==5)  temp = 0b010000010000001000000100010011100010001110; //New
    else if (i==6)  temp = 0b010010010010001000000011110011011010110010; //New
    else if (i==7)  temp = 0b010010011101001000000010010000011100100010; //New
    else if (i==8)  temp = 0b010010000001001000010100010001100100011010; //New
    else if (i==9)  temp = 0b010001000001001000001100010001100100000110; //New
    else if(i==10)  temp = 0b010000100011001111000000010000011100010010; //New
    else if(i==11)  temp = 0b010000011110000111000000010000001100010010; //New
    else if(i==12)  temp = 0b000000000000000000000000000000000100000000; //New
    else if(i==13)  temp = 0b000000000000000000000000000000000000000000; //New
    else if(i==14)  temp = 0b000000000000000000000000000000000000000000; //New
    else if(i==15)  temp = 0b000000000000000000000000000000000000000000; //New
 }

  temp = temp << shft;
  return temp;
 }

//Function for AMPM data
uint64_t ampm_fn( uint8_t ampm, uint8_t i, uint8_t shft ){
 //ampm = 0 means am, purbahno
 uint64_t temp = 0;

 if (ampm==0){
    if      (i==0)  temp = 0b0000000000000000010000000000000000;
    else if (i==1)  temp = 0b0000000000000001100000000000000000;
    else if (i==2)  temp = 0b0000000000000011000000000000000000;
    else if (i==3)  temp = 0b0111100111111111101111111111100000;
    else if (i==4)  temp = 0b1000010100000010010001000000000000;
    else if (i==5)  temp = 0b1110111100001110010011110000000000;
    else if (i==6)  temp = 0b0011001100110010010110001111000000;
    else if (i==7)  temp = 0b0010000100100010010000001001000000;
    else if (i==8)  temp = 0b0000000100010010010011110011000000;
    else if (i==9)  temp = 0b0000000100001010010000110000000000;
    else if(i==10)  temp = 0b0000000100000110010000001100000000;
    else if(i==11)  temp = 0b0000000100000010010000000110000000;
    else if(i==12)  temp = 0b0000001100000000000000000001011000;
    else if(i==13)  temp = 0b0000010001100000000000000000011000;
    else if(i==14)  temp = 0b0000011110000000000000000000001000;
    else if(i==15)  temp = 0b0000000000000000000000000000010000;
 }

 else{
    if      (i==0)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==1)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==2)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==3)  temp = 0b111111111111001111001111111111101111111111100000;
    else if (i==4)  temp = 0b000011100010010000101000000010010001000000000000;
    else if (i==5)  temp = 0b010010010010011101111000001110010001110000000000;
    else if (i==6)  temp = 0b010011001010000110011000110010010010001110000000;
    else if (i==7)  temp = 0b010000001010000100001001000010010000001001000000;
    else if (i==8)  temp = 0b001000001010000000001000100010010011110011000000;
    else if (i==9)  temp = 0b000100011010000000001000011010010000110000000000;
    else if(i==10)  temp = 0b000011110110000000001001000110010000001100000000;
    else if(i==11)  temp = 0b000000000010000000001000100010010000000010000000;
    else if(i==12)  temp = 0b000000000000000000000000000000000000000001011000;
    else if(i==13)  temp = 0b000000000000000000000000000000000000000000011000;
    else if(i==14)  temp = 0b000000000000000000000000000000000000000000001000;
    else if(i==15)  temp = 0b000000000000000000000000000000000000000000010000;
 }
  temp = temp << shft;
  return temp;
}

//matrices for Bengali months
uint64_t bn_months(uint8_t bn_m, uint8_t i, uint8_t shft ){
  
 uint64_t temp = 0;

//Baishakh, size = 33
 if (bn_m==1){
    if      (i==0)  temp = 0b000000000000000000000000000000000;
    else if (i==1)  temp = 0b111100000000000000000000000000000;
    else if (i==2)  temp = 0b000100000000000000000000000000000;
    else if (i==3)  temp = 0b001111111111110011011101010010011;
    else if (i==4)  temp = 0b001000000010001101110010011111110;
    else if (i==5)  temp = 0b010000001110011110110010000000110;
    else if (i==6)  temp = 0b010000110010011110010010000011010;
    else if (i==7)  temp = 0b010000100010011110010010000100010;
    else if (i==8)  temp = 0b010000011010000000010010000011010;
    else if (i==9)  temp = 0b010000000110000000010010000001110;
    else if(i==10)  temp = 0b011110000010000000010010000000010;
    else if(i==11)  temp = 0b001110000010000000010010000000010;
    else if(i==12)  temp = 0b000000000000000000000000000000000;
    else if(i==13)  temp = 0b000000000000000000000000000000000;
    else if(i==14)  temp = 0b000000000000000000000000000000000;
    else if(i==15)  temp = 0b000000000000000000000000000000000;
 }
 //JOISHTHO, Size = 23
 else if (bn_m==2){
    if      (i==0)  temp = 0b00000000000000000001100;
    else if (i==1)  temp = 0b11110000000000000001000;
    else if (i==2)  temp = 0b00010000000000000001000;
    else if (i==3)  temp = 0b00111111111111111111110;
    else if (i==4)  temp = 0b00100000010110000010010;
    else if (i==5)  temp = 0b01000100100001110011010;
    else if (i==6)  temp = 0b01000100100100010100111;
    else if (i==7)  temp = 0b01000100100110100011101;
    else if (i==8)  temp = 0b01000010011010100011001;
    else if (i==9)  temp = 0b01000010000010100010001;
    else if(i==10)  temp = 0b01111001000100100010011;
    else if(i==11)  temp = 0b00111000111100100001110;
    else if(i==12)  temp = 0b00000000000000010000000;
    else if(i==13)  temp = 0b00000000000000000000000;
    else if(i==14)  temp = 0b00000000000000000000000;
    else if(i==15)  temp = 0b00000000000000000000000;
 }
//ASHAR, size = 34
 else if (bn_m==3){ //ASHAR
    if      (i==0)  temp = 0b0000000000000000000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000000000000000000;
    else if (i==2)  temp = 0b0000000000000000000000000000000000;
    else if (i==3)  temp = 0b1111111111111011111111110111111111;
    else if (i==4)  temp = 0b0000100000100100110001001001000000;
    else if (i==5)  temp = 0b0100011100100100001001001001000000;
    else if (i==6)  temp = 0b0100111110100100011111001001001100;
    else if (i==7)  temp = 0b0100010010100100100011001001001110;
    else if (i==8)  temp = 0b0010000010100100111001001001000010;
    else if (i==9)  temp = 0b0001000110100100000101001001000010;
    else if(i==10)  temp = 0b0000111101100100000011001001100100;
    else if(i==11)  temp = 0b0000000000100100000001001000111000;
    else if(i==12)  temp = 0b0000000000100000000000000000000000;
    else if(i==13)  temp = 0b0000000000000000000000000000010000;
    else if(i==14)  temp = 0b0000000000000000000000000000010000;
    else if(i==15)  temp = 0b0000000000000000000000000000000000;
 }
//SRABON, size = 28
 else if (bn_m==4){
    if      (i==0)  temp = 0b0000000000000000000000000000;
    else if (i==1)  temp = 0b0000000000000000000000000000;
    else if (i==2)  temp = 0b0000000000000000000000000000;
    else if (i==3)  temp = 0b1110011011101111111100111011;
    else if (i==4)  temp = 0b0001100110010000001001000110;
    else if (i==5)  temp = 0b0001110110010000111001000110;
    else if (i==6)  temp = 0b0011111010010011001001110010;
    else if (i==7)  temp = 0b0011010010010010001000110010;
    else if (i==8)  temp = 0b0000000010010001101000000010;
    else if (i==9)  temp = 0b0100000010010000011000000010;
    else if(i==10)  temp = 0b0100011110010000001000000010;
    else if(i==11)  temp = 0b0011100010010000001000000010;
    else if(i==12)  temp = 0b0000000000000000000000000000;
    else if(i==13)  temp = 0b0000000000000000000000000000;
    else if(i==14)  temp = 0b0000000000000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000000000000;
 }
//BHADRO, size=23
 else if (bn_m==5){
    if      (i==0)  temp = 0b00000000000000000000000;
    else if (i==1)  temp = 0b00000000000000000000000;
    else if (i==2)  temp = 0b00000000000000000000000;
    else if (i==3)  temp = 0b11111111111011111111111;
    else if (i==4)  temp = 0b00000000000100001000000;
    else if (i==5)  temp = 0b00001000000100001001100;
    else if (i==6)  temp = 0b01011101000100001111000;
    else if (i==7)  temp = 0b01011110100100001001000;
    else if (i==8)  temp = 0b01000000100100101001000;
    else if (i==9)  temp = 0b00100000100100100001000;
    else if(i==10)  temp = 0b00010001000100100111000;
    else if(i==11)  temp = 0b00001110000100011101000;
    else if(i==12)  temp = 0b00000000000000000000100;
    else if(i==13)  temp = 0b00000000000000000000000;
    else if(i==14)  temp = 0b00000000000000000000000;
    else if(i==15)  temp = 0b00000000000000000000000;
                           0b0000000000000000000;
 }
//ASHIN, size = 35
 else if (bn_m==6){
    if      (i==0)  temp = 0b00000000000000000111111000000000000;
    else if (i==1)  temp = 0b00000000000000001000000110000000000;
    else if (i==2)  temp = 0b00000000000000001000000000000000000;
    else if (i==3)  temp = 0b11111111111110111111001101111111111;
    else if (i==4)  temp = 0b00001000001001001000110011000000010;
    else if (i==5)  temp = 0b01000111001001001001111011000000010;
    else if (i==6)  temp = 0b01001111101001001001111101001111010;
    else if (i==7)  temp = 0b01000100101001001001101011001101010;
    else if (i==8)  temp = 0b00100000101001001000000101001110110;
    else if (i==9)  temp = 0b00010001101001001000001001000000110;
    else if(i==10)  temp = 0b00001111011001001000001111000000010;
    else if(i==11)  temp = 0b00000000001001001000000001000000010;
    else if(i==12)  temp = 0b00000000000000000000000000000000000;
    else if(i==13)  temp = 0b00000000000000000000000000000000000;
    else if(i==14)  temp = 0b00000000000000000000000000000000000;
    else if(i==15)  temp = 0b00000000000000000000000000000000000;
 }
//KARTIK, size = 38
 else if (bn_m==7){
    if      (i==0)  temp = 0b00000000000000001111110010000000000000;
    else if (i==1)  temp = 0b00000000000000010000001100000000000000;
    else if (i==2)  temp = 0b00000000000000010000011000000000000000;
    else if (i==3)  temp = 0b11111111111101111111111111111111111111;
    else if (i==4)  temp = 0b00000010000010010000000000000000100000;
    else if (i==5)  temp = 0b00001111100010010000011110000011111000;
    else if (i==6)  temp = 0b00110010011010010010011111001100100110;
    else if (i==7)  temp = 0b01000010001010010010011001010000100010;
    else if (i==8)  temp = 0b00110010111010010010000001001100101110;
    else if (i==9)  temp = 0b00001110110010010001000001000011101100;
    else if(i==10)  temp = 0b00000110000010010000100010000001100000;
    else if(i==11)  temp = 0b00000010000010010000011100000000100000;
    else if(i==12)  temp = 0b00000000000000000000000000000000000000;
    else if(i==13)  temp = 0b00000000000000000000000000000000000000;
    else if(i==14)  temp = 0b00000000000000000000000000000000000000;
    else if(i==15)  temp = 0b00000000000000000000000000000000000000;
 }
//OGROHAYON, size = 48
 else if (bn_m==8){
    if      (i==0)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==1)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==2)  temp = 0b000000000000000000000000000000000000000000000000;
    else if (i==3)  temp = 0b111111111111000111011111111111011111111100111011;
    else if (i==4)  temp = 0b000010000010001000110000100000100110001001000110;
    else if (i==5)  temp = 0b010001110010001110110001111100100001001001000110;
    else if (i==6)  temp = 0b010011111010000010010001000100100011101001110010;
    else if (i==7)  temp = 0b010001001010000010010000001100100100001000110010;
    else if (i==8)  temp = 0b001000001010000100010011110000100111001000000010;
    else if (i==9)  temp = 0b000100011010010000010000011000100000101000000010;
    else if(i==10)  temp = 0b000011110110010011110000000100100001011000000010;
    else if(i==11)  temp = 0b000000000010001100010000000010100001001000000010;
    else if(i==12)  temp = 0b000000000000000000000000000000000000000000000000;
    else if(i==13)  temp = 0b000000000000000000000000000000000000000000000000;
    else if(i==14)  temp = 0b000000000000000000000000000000000000000000000000;
    else if(i==15)  temp = 0b000000000000000000000000000000000000000000000000;
 }
//POUSH, size = 25
 else if (bn_m==9){
    if      (i==0)  temp = 0b0000000000000000000000000;
    else if (i==1)  temp = 0b0000000001001110000000000;
    else if (i==2)  temp = 0b0000000001110001000000000;
    else if (i==3)  temp = 0b0111001111001111111111111;
    else if (i==4)  temp = 0b0100010000101001001100010;
    else if (i==5)  temp = 0b1000011101111001000010010;
    else if (i==6)  temp = 0b1000000110011001000111110;
    else if (i==7)  temp = 0b1000000100001001001000110;
    else if (i==8)  temp = 0b1000000000001001001110010;
    else if (i==9)  temp = 0b1000000000001001000001010;
    else if(i==10)  temp = 0b1111000000001001000000110;
    else if(i==11)  temp = 0b0111000000001001000000010;
    else if(i==12)  temp = 0b0000000000000000000000000;
    else if(i==13)  temp = 0b0000000000000000000000000;
    else if(i==14)  temp = 0b0000000000000000000000000;
    else if(i==15)  temp = 0b0000000000000000000000000;
 }

 else if (bn_m==10){
// MAGH, size = 20
    if      (i==0)  temp = 0b00000000000000000000; //New
    else if (i==1)  temp = 0b00000000000000000000; //New
    else if (i==2)  temp = 0b00000000000000000000; //New
    else if (i==3)  temp = 0b11111111110111111111; //New
    else if (i==4)  temp = 0b01100001001001000010; //New
    else if (i==5)  temp = 0b00010001001001000010; //New
    else if (i==6)  temp = 0b00001001001000111010; //New
    else if (i==7)  temp = 0b00001001001000110010; //New
    else if (i==8)  temp = 0b00111001001000110010; //New
    else if (i==9)  temp = 0b00111111001000001110; //New
    else if(i==10)  temp = 0b00000011001000000110; //New
    else if(i==11)  temp = 0b00000001001000000010; //New
    else if(i==12)  temp = 0b00000000000000000000; //New
    else if(i==13)  temp = 0b00000000000000000000; //New
    else if(i==14)  temp = 0b00000000000000000000; //New
    else if(i==15)  temp = 0b00000000000000000000; //New
 }

 else if (bn_m==11){
 // FALGUN, size = 33
    if      (i==0)  temp = 0b000000000000000000000000000000000; //New
    else if (i==1)  temp = 0b000000000000000000000000000000000; //New
    else if (i==2)  temp = 0b000000000000000000000000000000000; //New
    else if (i==3)  temp = 0b111111111111011111111111111111111; //New
    else if (i==4)  temp = 0b011000000000100000000100000000010; //New
    else if (i==5)  temp = 0b001100111100100011111100000000010; //New
    else if (i==6)  temp = 0b001100100010100100100100001111010; //New
    else if (i==7)  temp = 0b010000101010100111100100001101010; //New
    else if (i==8)  temp = 0b011100101100101011110100001110110; //New
    else if (i==9)  temp = 0b000011100000101011101010000000110; //New
    else if(i==10)  temp = 0b000001100000101010100010000000010; //New
    else if(i==11)  temp = 0b000000100000100101100010000000010; //New
    else if(i==12)  temp = 0b000000000000000010000010000000000; //New
    else if(i==13)  temp = 0b000000000000000001000010000000000; //New
    else if(i==14)  temp = 0b000000000000000000111100000000000; //New
    else if(i==15)  temp = 0b000000000000000000000000000000000; //New
 }

 else if (bn_m==12){
    // CHOITRO, size = 22
    if      (i==0)  temp = 0b0000000000000000000000; //New
    else if (i==1)  temp = 0b1110000000000000000000; //New
    else if (i==2)  temp = 0b0010000000000000000000; //New
    else if (i==3)  temp = 0b0011111111111111111111; //New
    else if (i==4)  temp = 0b0010001000000000000000; //New
    else if (i==5)  temp = 0b0100001100000000001110; //New
    else if (i==6)  temp = 0b0100001011100000110010; //New
    else if (i==7)  temp = 0b0100001000100000101010; //New
    else if (i==8)  temp = 0b0100001000100100111010; //New
    else if (i==9)  temp = 0b0100001001000100000010; //New
    else if(i==10)  temp = 0b0111101001000100011110; //New
    else if(i==11)  temp = 0b0011100110000011100010; //New
    else if(i==12)  temp = 0b0000000000000000000000; //New
    else if(i==13)  temp = 0b0000000000000000000000; //New
    else if(i==14)  temp = 0b0000000000000000000000; //New
    else if(i==15)  temp = 0b0000000000000000000000; //New
 }

  temp = temp << shft;
  return temp;
}

//The function to display digits
uint64_t digits_fn( uint8_t dig_it, uint8_t i, uint8_t shft ){

uint64_t temp=0;

 if (dig_it==0){
  // ZERO
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00000000; //New
    else if (i==4)  temp = 0b00111100; //New
    else if (i==5)  temp = 0b01000010; //New
    else if (i==6)  temp = 0b10000001; //New
    else if (i==7)  temp = 0b10000001; //New
    else if (i==8)  temp = 0b10000001; //New
    else if (i==9)  temp = 0b10000001; //New
    else if(i==10)  temp = 0b01000010; //New
    else if(i==11)  temp = 0b00111100; //New
    else if(i==12)  temp = 0b00000000; //New
    else if(i==13)  temp = 0b00000000; //New
}
  
 else if (dig_it==1){
    // ONE
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b01000000; //New
    else if (i==4)  temp = 0b00100000; //New
    else if (i==5)  temp = 0b00010000; //New
    else if (i==6)  temp = 0b00001000; //New
    else if (i==7)  temp = 0b00000100; //New
    else if (i==8)  temp = 0b00000010; //New
    else if (i==9)  temp = 0b00000010; //New
    else if(i==10)  temp = 0b00110010; //New
    else if(i==11)  temp = 0b01110100; //New
    else if(i==12)  temp = 0b00111000; //New
    else if(i==13)  temp = 0b00000000; //New
}
    
 else if (dig_it==2){
    // TWO
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b01000000; //New
    else if (i==4)  temp = 0b00111100; //New
    else if (i==5)  temp = 0b00000010; //New
    else if (i==6)  temp = 0b00000010; //New
    else if (i==7)  temp = 0b00001100; //New
    else if (i==8)  temp = 0b01111000; //New
    else if (i==9)  temp = 0b00010000; //New
    else if(i==10)  temp = 0b00001000; //New
    else if(i==11)  temp = 0b00000100; //New
    else if(i==12)  temp = 0b00000011; //New
    else if(i==13)  temp = 0b00000000; //New
}
    
 else if (dig_it==3){
// THREE
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00000000; //New
    else if (i==4)  temp = 0b00001100; //New
    else if (i==5)  temp = 0b00011110; //New
    else if (i==6)  temp = 0b10011101; //New
    else if (i==7)  temp = 0b10000001; //New
    else if (i==8)  temp = 0b10000001; //New
    else if (i==9)  temp = 0b01000001; //New
    else if(i==10)  temp = 0b00100010; //New
    else if(i==11)  temp = 0b00011100; //New
    else if(i==12)  temp = 0b00000000; //New
    else if(i==13)  temp = 0b00000000; //New
}
    
 else if (dig_it==4){
// FOUR
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00111100; //New
    else if (i==4)  temp = 0b01000010; //New
    else if (i==5)  temp = 0b01000010; //New
    else if (i==6)  temp = 0b01000010; //New
    else if (i==7)  temp = 0b01000010; //New
    else if (i==8)  temp = 0b00111100; //New
    else if (i==9)  temp = 0b01000010; //New
    else if(i==10)  temp = 0b01000010; //New
    else if(i==11)  temp = 0b01000010; //New
    else if(i==12)  temp = 0b00111100; //New
    else if(i==13)  temp = 0b00000000; //New
}
  
 else if (dig_it==5){
// FIVE 
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00001000; //New
    else if (i==4)  temp = 0b00011000; //New
    else if (i==5)  temp = 0b00101000; //New
    else if (i==6)  temp = 0b01100110; //New
    else if (i==7)  temp = 0b01000100; //New
    else if (i==8)  temp = 0b01001000; //New
    else if (i==9)  temp = 0b01001000; //New
    else if(i==10)  temp = 0b01001000; //New
    else if(i==11)  temp = 0b00101100; //New
    else if(i==12)  temp = 0b00011110; //New
    else if(i==13)  temp = 0b00000000; //New
}

 else if (dig_it==6){
 // SIX
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00100000; //New
    else if (i==4)  temp = 0b00010000; //New
    else if (i==5)  temp = 0b00010000; //New
    else if (i==6)  temp = 0b10010110; //New
    else if (i==7)  temp = 0b10011001; //New
    else if (i==8)  temp = 0b01000001; //New
    else if (i==9)  temp = 0b01000001; //New
    else if(i==10)  temp = 0b00100010; //New
    else if(i==11)  temp = 0b00011100; //New
    else if(i==12)  temp = 0b00000000; //New
    else if(i==13)  temp = 0b00000000; //New
}
  
 else if (dig_it==7){
// SEVEN
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00011000; //New
    else if (i==4)  temp = 0b00100100; //New
    else if (i==5)  temp = 0b01000100; //New
    else if (i==6)  temp = 0b01000100; //New
    else if (i==7)  temp = 0b00111100; //New
    else if (i==8)  temp = 0b00000100; //New
    else if (i==9)  temp = 0b00000100; //New
    else if(i==10)  temp = 0b00000100; //New
    else if(i==11)  temp = 0b00000110; //New
    else if(i==12)  temp = 0b00000110; //New
    else if(i==13)  temp = 0b00000000; //New
}
  
 else if (dig_it==8){
// EIGHT
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b00000000; //New
    else if (i==4)  temp = 0b11000000; //New
    else if (i==5)  temp = 0b01000000; //New
    else if (i==6)  temp = 0b01000001; //New
    else if (i==7)  temp = 0b01111111; //New
    else if (i==8)  temp = 0b01000100; //New
    else if (i==9)  temp = 0b01000100; //New
    else if(i==10)  temp = 0b01000100; //New
    else if(i==11)  temp = 0b00111000; //New
    else if(i==12)  temp = 0b00000000; //New
    else if(i==13)  temp = 0b00000000; //New
}

 else if (dig_it==9){
// NINE
    if      (i==2)  temp = 0b00000000; //New
    else if (i==3)  temp = 0b01000000; //New
    else if (i==4)  temp = 0b01000000; //New
    else if (i==5)  temp = 0b00100000; //New
    else if (i==6)  temp = 0b00011100; //New
    else if (i==7)  temp = 0b00000010; //New
    else if (i==8)  temp = 0b01110001; //New
    else if (i==9)  temp = 0b10001001; //New
    else if(i==10)  temp = 0b11101001; //New
    else if(i==11)  temp = 0b01100110; //New
    else if(i==12)  temp = 0b00000000; //New
    else if(i==13)  temp = 0b00000000; //New
}

 else if (dig_it==10){
  //COMMA
    if      (i==12)  temp = 0b011;
    else if (i==13)  temp = 0b011;
    else if (i==14)  temp = 0b001;
    else if (i==15)  temp = 0b010;}

  temp = temp << shft;
  return temp;
}

//The function to dispay week days
uint32_t week_fn( uint8_t wk_day, uint8_t i, uint8_t shft ){
  
 uint32_t temp = 0;

 if (wk_day==1){
  // SAT, size = 23    
    if      (i==0)  temp = 0b00000000000111111110000; //New
    else if (i==1)  temp = 0b00000000001000000001000; //New
    else if (i==2)  temp = 0b00000000001000000000000; //New
    else if (i==3)  temp = 0b11011101111111111111100; //New
    else if (i==4)  temp = 0b00110111001000000001000; //New
    else if (i==5)  temp = 0b01111011001000000001000; //New
    else if (i==6)  temp = 0b11111001001000111001000; //New
    else if (i==7)  temp = 0b11011001001001110101000; //New
    else if (i==8)  temp = 0b00000001001000110011000; //New
    else if (i==9)  temp = 0b00000001001000000011000; //New
    else if(i==10)  temp = 0b00000001001000000001000; //New
    else if(i==11)  temp = 0b00000001001000000001000; //New
    else if(i==12)  temp = 0b00000000000000000000011; //New
    else if(i==13)  temp = 0b00000000000000000000001; //New
    else if(i==14)  temp = 0b00000000000000000000001; //New
    else if(i==15)  temp = 0b00000000000000000000010; //New
 }

 else if (wk_day==2){
  // SUN, size = 21
    if      (i==0)  temp = 0b000000000011111111000; //New
    else if (i==1)  temp = 0b000000000100000001000; //New
    else if (i==2)  temp = 0b000000000100000000000; //New
    else if (i==3)  temp = 0b111111111111111111100; //New
    else if (i==4)  temp = 0b000000100100000001000; //New
    else if (i==5)  temp = 0b000011100100000111000; //New
    else if (i==6)  temp = 0b001100100100011001000; //New
    else if (i==7)  temp = 0b010000100100100001000; //New
    else if (i==8)  temp = 0b001100100100011001000; //New
    else if (i==9)  temp = 0b000011100100000111000; //New
    else if(i==10)  temp = 0b010001100100000011000; //New
    else if(i==11)  temp = 0b010000100100000001000; //New
    else if(i==12)  temp = 0b000000000000000000011; //New
    else if(i==13)  temp = 0b000000000000000000001; //New
    else if(i==14)  temp = 0b000000000000000000001; //New
    else if(i==15)  temp = 0b000000000000000000010; //New
 }

 else if (wk_day==3){
  // MON, size = 27
    if      (i==0)  temp = 0b000000000000000000000000000; //New
    else if (i==1)  temp = 0b000000000000000000000000000; //New
    else if (i==2)  temp = 0b000000000000000000000000000; //New
    else if (i==3)  temp = 0b001111111111110111111111100; //New
    else if (i==4)  temp = 0b010001100001001001100001000; //New
    else if (i==5)  temp = 0b110000110001001000010001000; //New
    else if (i==6)  temp = 0b100000010111001000001001000; //New
    else if (i==7)  temp = 0b100000001001001000001001000; //New
    else if (i==8)  temp = 0b100001010001001000111001000; //New
    else if (i==9)  temp = 0b100001100001001000110111000; //New
    else if(i==10)  temp = 0b111100000001001000000011000; //New
    else if(i==11)  temp = 0b011100000001001000000001000; //New
    else if(i==12)  temp = 0b000000000000000000000000011; //New
    else if(i==13)  temp = 0b000000000000000000000000001; //New
    else if(i==14)  temp = 0b000000000000000000000000001; //New
    else if(i==15)  temp = 0b000000000000000000000000010; //New
 }

 else if (wk_day==4){
  // Tue, sine = 30
    if      (i==0)  temp = 0b000000000000000000000000000000; //New
    else if (i==1)  temp = 0b000000000000000000000000000000; //New
    else if (i==2)  temp = 0b000000000000000000000000000000; //New
    else if (i==3)  temp = 0b111111111111111111111111111000; //New
    else if (i==4)  temp = 0b011000010010001001000000001000; //New
    else if (i==5)  temp = 0b000100010011110111000110111000; //New
    else if (i==6)  temp = 0b000010010011101001001001001000; //New
    else if (i==7)  temp = 0b000010010011001111001010001000; //New
    else if (i==8)  temp = 0b001110010000001010001110001000; //New
    else if (i==9)  temp = 0b001101110000011010000110001000; //New
    else if(i==10)  temp = 0b000000110001110010000000001000; //New
    else if(i==11)  temp = 0b000000010000000010000000001000; //New
    else if(i==12)  temp = 0b000000000000000000000000000011; //New
    else if(i==13)  temp = 0b000000000000000000000000000001; //New
    else if(i==14)  temp = 0b000000000000000000000000000001; //New
    else if(i==15)  temp = 0b000000000000000000000000000010; //New
}

 else if (wk_day==5){
  // Wed, size = 18
    if      (i==0)  temp = 0b000000000000000000; //New
    else if (i==1)  temp = 0b000000000000000000; //New
    else if (i==2)  temp = 0b000000000000000000; //New
    else if (i==3)  temp = 0b111111110111001100; //New
    else if (i==4)  temp = 0b000000100101001000; //New
    else if (i==5)  temp = 0b000011100110111000; //New
    else if (i==6)  temp = 0b001100100011001000; //New
    else if (i==7)  temp = 0b010000100100001000; //New
    else if (i==8)  temp = 0b001100100011001000; //New
    else if (i==9)  temp = 0b000011100000111000; //New
    else if(i==10)  temp = 0b000001100000011000; //New
    else if(i==11)  temp = 0b000000100000001000; //New
    else if(i==12)  temp = 0b000111100000000011; //New
    else if(i==13)  temp = 0b000101011000000001; //New
    else if(i==14)  temp = 0b000111001000000001; //New
    else if(i==15)  temp = 0b000000000000000010; //New
}
 else if (wk_day==6){
  // Thurs, size = 24
    if      (i==0)  temp = 0b000000000000000000000000; //New
    else if (i==1)  temp = 0b000000000000000000000000; //New
    else if (i==2)  temp = 0b000000000000000000000000; //New
    else if (i==3)  temp = 0b111111111111111100000000; //New
    else if (i==4)  temp = 0b000000100001000000110000; //New
    else if (i==5)  temp = 0b000011100011111001001000; //New
    else if (i==6)  temp = 0b001100100010001001001000; //New
    else if (i==7)  temp = 0b010000100000011000110000; //New
    else if (i==8)  temp = 0b001100100111100000000000; //New
    else if (i==9)  temp = 0b000011100000110000110000; //New
    else if(i==10)  temp = 0b000001100000001001001000; //New
    else if(i==11)  temp = 0b000001100000000101001000; //New
    else if(i==12)  temp = 0b000010000000000000110011; //New
    else if(i==13)  temp = 0b000001000000000000000001; //New
    else if(i==14)  temp = 0b000000100000000000000001; //New
    else if(i==15)  temp = 0b000000000000000000000010; //New
}

 else if (wk_day==7){ 
// Friday, size = 27
    if      (i==0)  temp = 0b000000000000000000000000000; //New
    else if (i==1)  temp = 0b000000000000000000000000000; //New
    else if (i==2)  temp = 0b000000000000000000000000000; //New
    else if (i==3)  temp = 0b011101101001111111111111100; //New
    else if (i==4)  temp = 0b010110111000000000000000000; //New
    else if (i==5)  temp = 0b101111010100000001111111000; //New
    else if (i==6)  temp = 0b101101100100000010010000100; //New
    else if (i==7)  temp = 0b010000011100000100010000100; //New
    else if (i==8)  temp = 0b010000011100000111010010100; //New
    else if (i==9)  temp = 0b001000000100100011010011000; //New
    else if(i==10)  temp = 0b000110000100100011110000000; //New
    else if(i==11)  temp = 0b000001111000011100010000000; //New
    else if(i==12)  temp = 0b000000000000000000010000011; //New
    else if(i==13)  temp = 0b000000000000000000000000001; //New
    else if(i==14)  temp = 0b000000000000000000000000001; //New
    else if(i==15)  temp = 0b000000000000000000000000010; //New
}

  temp = temp << shft;
  return temp;
 }

/////////// /Let there be Light!!! ///////////////////////////////////////////////////////////////////////////////////////
void populate_cells()
{
  //uint8_t en_m = 7;
  //uint8_t bn_m = 3;
  //uint8_t ampm = 1;
  //ampm size: 34 if ampm=0 (AM), 48 if ampm=1 (PM)
  uint8_t ampm_size = 34 + 14*ampm;
  
  //Arrays for display
  uint64_t time_mat = 0;
  uint64_t ampm_mat = 0;
  uint64_t ban_dy_mat = 0; //size 64
  uint64_t ban_m_mat = 0; //size 64
  uint64_t eng_dy_mat = 0; //size 64
  uint64_t eng_m_mat = 0; //size 64
  uint32_t week_mat = 0; //size 32

  uint8_t week_size = wk_size[week] + 3; //3 spaces after week

  uint32_t disp_size = ampm_size + week_size + d_size + bn_m_size[bn_m] + y_size + 3 + 3 + d_size + en_m_size[en_m] + y_size + 6;
  //uint8_t disp_size = ampm_size + d_size + bn_m_size[bn_m] + y_size + 3 + 3 + d_size + en_m_size[en_m] + y_size + 6;

  for(int i=0;i<16;i++){

      //Debugging

//      bn_m=1;
//      en_m=1;
//      week = 1;
//
//      bn_date[0]=2;
//      bn_date[1]=3;
//      
//      bn_year[0]=1;
//      bn_year[1]=9;
//      bn_year[2]=4;
//      bn_year[3]=3;
//
//      en_date[0]=1;
//      bn_date[1]=5;
//
//      en_year[0]=2;
//      en_year[1]=0;
//      en_year[2]=3;
//      en_year[3]=4;
//
//      tim_e[0]=1;
//      tim_e[1]=1;
//      tim_e[2]=4;
//      tim_e[3]=1;
//
//      ampm=1;
      //Debugging
      
      //AMPM
      ampm_mat = ampm_fn( ampm, i, (64 - ampm_size) );
      
      //MONTHS
      eng_m_mat = en_months( en_m, i, (64 - en_m_size[en_m] - 0));
      ban_m_mat = bn_months( bn_m, i, (64 - bn_m_size[bn_m] - 0));
      week_mat  =   week_fn( week, i, (32 - wk_size[week]) );

      eng_dy_mat=0;
      ban_dy_mat=0;
      time_mat=0;

      //TIME DOTS
      if((i==4 || i==5 || i==10 || i==11) && second_loop<6 ){
        time_mat = 1;
        time_mat = time_mat << (64 - 2*N1 - 3);
      }
      
      if (i>1 && i<14){

        //TIME
        time_mat = time_mat | digits_fn( tim_e[0], i, (64 - N1) );
        time_mat = time_mat | digits_fn( tim_e[1], i, (64 - 2*N1 - 1) );
        time_mat = time_mat | digits_fn( tim_e[2], i, (64 - 3*N1 - 4) );
        time_mat = time_mat | digits_fn( tim_e[3], i, (64 - 4*N1 - 5) );

        //BENGALI DATE
        ban_dy_mat =              digits_fn( bn_date[0], i, (64 - (N1 + 0)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_date[1], i, (64 - (N1 + N1 + 1)) );

        //BENGALI YEAR
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[0], i, (64 - (N1 + 2*N1 + 5)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[1], i, (64 - (N1 + 3*N1 + 6)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[2], i, (64 - (N1 + 4*N1 + 7)) );
        ban_dy_mat = ban_dy_mat | digits_fn( bn_year[3], i, (64 - (N1 + 5*N1 + 8)) );
        
        //ENGLISH DATE
        eng_dy_mat =              digits_fn( en_date[0], i, (64 - (N1 + 0)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_date[1], i, (64 - (N1 + N1 + 1)) );

        //ENGLISH YEAR
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[0], i, (64 - (N1 + 2*N1 + 5)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[1], i, (64 - (N1 + 3*N1 + 6)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[2], i, (64 - (N1 + 4*N1 + 7)) );
        eng_dy_mat = eng_dy_mat | digits_fn( en_year[3], i, (64 - (N1 + 5*N1 + 8)) );
      }

      //COMMA AFTER BENGALI YEAR
      if(i>10) ban_dy_mat = ban_dy_mat | digits_fn( 10, i, (64 - (N1 + 5*N1 + 8 + 3)) );
        
    
    //The seperating Line
    uint8_t offset = 39;
    dmd.setPixel(offset-1,i,GRAPHICS_ON);
    
    for(int j=0;j<38;j++)
    { 
      //TIME
      if ( (time_mat >> (63 - j)) & 0b1 )
        dmd.setPixel(j,i,GRAPHICS_ON);
      else
        dmd.setPixel(j,i,GRAPHICS_OFF); 

      //////////////////////////////////// SCROLLING DATE AND TIME ///////////////////////////////////
      if (j<25){
      uint32_t col_ind = (j+lo_op)%disp_size;
      uint8_t flag = 0;

      //disp_size = ampm_size + week_size + d_size + bn_m_size[bn_m] + y_size + 3 + 3 + d_size + en_m_size[en_m] + y_size + 6;
      
      
      if (col_ind < ampm_size)
        flag = (ampm_mat >> (63-col_ind) ) & 0b1;

      //WEEK DAY DISPLAY
      else if (col_ind < ampm_size + week_size)
        flag = (week_mat >> (32 - col_ind + ampm_size)) & 0b1;
      
      //BANGLA DATE AND TIME 
      else if (col_ind < (ampm_size + week_size + d_size))
        flag = (ban_dy_mat >> (63-col_ind +ampm_size + week_size) ) & 0b1;
      
      else if (col_ind < (ampm_size + week_size + d_size + bn_m_size[bn_m]))
        flag = (ban_m_mat >> (63-col_ind +ampm_size + week_size +d_size) ) & 0b1;
        
      else if (col_ind < (ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+3))
        flag = (ban_dy_mat >> (63-col_ind +ampm_size + week_size +bn_m_size[bn_m]) ) & 0b1;

      //ENGLISH DATE AND TIME 
      else if (col_ind < (ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+6+d_size))
        flag = (eng_dy_mat >> (63-col_ind +ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+6) ) & 0b1;

      else if (col_ind < (ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+6+d_size + en_m_size[en_m]))
        flag = (eng_m_mat >> (63-col_ind +ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+6+d_size) ) & 0b1;

      else if (col_ind < (ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+6+d_size + en_m_size[en_m] + y_size + 6))
        flag = (eng_dy_mat >> (63-col_ind +ampm_size + week_size + d_size + bn_m_size[bn_m]+y_size+6+en_m_size[en_m]) ) & 0b1;
      
      if (flag)
        dmd.setPixel(j+offset,i,GRAPHICS_ON);
      else
        dmd.setPixel(j+offset,i,GRAPHICS_OFF); 
      }
    }

    
  }
//Now wait and scroll
delay(100);
second_loop = (second_loop + 1)%10;
lo_op = (lo_op+1)%disp_size;
}

/////////////////////////////////////////////////////// MATRIX MANIPULATION /////////////////////////////////////////////////////////


