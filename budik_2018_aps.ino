//importovanie knižníc
#include <Tone.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ctype.h>

#define SONGPAUSE 1000 //prestávka medzi jednotlivými prehrávaniami pesničky v milisekundách
#define TONEPAUSE 10 //prestávka medzi jednotlivými tónmi pesničky v milisekundách

//tvorba premenných
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27,16,2); //lcd displej s adresou 0x27, 16 znakmi v 2 riadkoch
byte icon1[8]={B00000,B00100,B01110,B01110,B01110,B11111,B00100,B00000}; //vlastný zobraziteľný znak budíka

typedef enum
{
   CMD_NONE,
   CMD_SET_DATE,
   CMD_SET_TIME,
   CMD_SET_ALARM,
   CMD_SET_ALARM_TIME,
   CMD_ALARM_TURN_OFF,
   CMD_SET_ALARM_SONG,
   CMD_COUNT
}TCmd; //enumeračný typ - hodnoty uvádzajú o aký príkaz používateľa sa jedná

TCmd cmd = CMD_NONE;

typedef enum
{
  TONE_OP_NONE,
  TONE_OP_PLAYING,
  TONE_OP_WAITING,
  TONE_OP_WAITING_AFTER_SONG
}TToneOp; //enumeračný typ - hodnoty uvádzajú v akom stave je práve prehrávaný tón pesničky

typedef struct
{
  int note;
  int numDur;
  int denDur;
}myTone; //štruktúra tónu - skladá sa z prehrávanej noty, a dĺžky(numDur čitateľ, denDur menovateľ)

typedef struct
{
  int tempo; //trvanie jedneho taktu v ms
  myTone *pTones;
  int tonesCount;
}mySong; //štruktúra piesne - skladá sa z dĺžky jedného taktu, odkazu na prehrávaný tón a počtu nôt

//definovanie piesne ako polia štruktúr tónov
myTone song1Tones[] = {
  //prvy takt
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,2},
  
  //druhy takt
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,2},
  
  //treti takt
  {NOTE_E4,1,4},
  {NOTE_G4,1,4},
  {NOTE_C4,3,8},
  {NOTE_D4,1,8},
  
  //stvrty takt
  {NOTE_E4,1,1},
  
  //piaty takt
  {NOTE_F4,1,4},
  {NOTE_F4,1,4},
  {NOTE_F4,3,8},
  {NOTE_F4,1,8},

  //siesty takt
  {NOTE_F4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,8},
  {NOTE_E4,1,8},
  
  //siedmy takt
  {NOTE_E4,1,4},
  {NOTE_D4,1,4},
  {NOTE_D4,1,4},
  {NOTE_E4,1,4},

  //osmy takt
  {NOTE_D4,1,2},
  {NOTE_G4,1,2},

  //deviaty takt
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,2},
  
  //desiaty takt
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,2},
  
  //jedenasty takt
  {NOTE_E4,1,4},
  {NOTE_G4,1,4},
  {NOTE_C4,3,8},
  {NOTE_D4,1,8},
  
  //dvanasty takt
  {NOTE_E4,1,1},
  
  //trinasty takt
  {NOTE_F4,1,4},
  {NOTE_F4,1,4},
  {NOTE_F4,1,4},
  {NOTE_F4,1,4},

  //strnasty takt
  {NOTE_F4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_E4,1,8},
  {NOTE_E4,1,8},

  //patnasty takt
  {NOTE_G4,1,4},
  {NOTE_G4,1,4},
  {NOTE_F4,1,4},
  {NOTE_D4,1,4},

  //sestnasty takt
  {NOTE_C4,1,1}
  }; 

myTone song2Tones[] = {
  //prvy takt
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_F4,1,4},
  {NOTE_G4,1,4},

  //druhy takt
  {NOTE_G4,1,4},
  {NOTE_F4,1,4},
  {NOTE_E4,1,4},
  {NOTE_D4,1,4},

  //treti takt
  {NOTE_C4,1,4},
  {NOTE_C4,1,4},
  {NOTE_D4,1,4},
  {NOTE_E4,1,4},

  //stvrty takt
  {NOTE_E4,3,8},
  {NOTE_D4,1,8},
  {NOTE_D4,1,2},

  //piaty takt
  {NOTE_E4,1,4},
  {NOTE_E4,1,4},
  {NOTE_F4,1,4},
  {NOTE_G4,1,4},

  //siesty takt
  {NOTE_G4,1,4},
  {NOTE_F4,1,4},
  {NOTE_E4,1,4},
  {NOTE_D4,1,4},

  //siedmy takt
  {NOTE_C4,1,4},
  {NOTE_C4,1,4},
  {NOTE_D4,1,4},
  {NOTE_E4,1,4},

  //osmy takt
  {NOTE_D4,3,8},
  {NOTE_C4,1,8},
  {NOTE_C4,1,2},
};

myTone song3Tones[] = {
  //prvy takt
  {NOTE_A3,1,2},
  {NOTE_A3,1,2},
  {NOTE_B3,1,2},
  {NOTE_A3,1,2},
  {NOTE_D4,1,1},
  {NOTE_CS4,1,1},
  
  //druhy takt
  {NOTE_A3,1,2},
  {NOTE_A3,1,2},
  {NOTE_B3,1,2},
  {NOTE_A3,1,2},
  {NOTE_E4,1,1},
  {NOTE_D4,1,1},

  //treti takt
  {NOTE_A3,1,2},
  {NOTE_A3,1,2},
  {NOTE_A4,1,2},
  {NOTE_FS4,1,2},
  {NOTE_D4,1,1},
  {NOTE_CS4,1,2},
  {NOTE_B3,1,2},

  //stvrty takt
  {NOTE_G4,1,2},
  {NOTE_G4,1,2},
  {NOTE_FS4,1,2},
  {NOTE_D4,1,2},
  {NOTE_E4,1,1},
  {NOTE_D4,1,1}
};

mySong song1 =
{
  1000,
  song1Tones,
  sizeof(song1Tones)/sizeof(myTone)
}; //vytvorenie pesničky - dĺžka taktu 1 sekunda, prehrávajú sa tóny z polia song1Tones, počet nôt

mySong song2 =
{
  2000,
  song2Tones,
  sizeof(song2Tones)/sizeof(myTone)
};

mySong song3 =
{
  800,
  song3Tones,
  sizeof(song3Tones)/sizeof(myTone)
};

mySong selectedSong = song1;
boolean alarmState = false;
boolean isAlarmSet = false;
boolean alarmStarted = false;
String paramString = ""; //budeme do neho ukladať zadaný dátum, čas, voľbu pesničku od používateľa
int buzzerPin = 10;

//premenne typu DateTime - slúžia na porovnanie, či došlo k zmene času od poslednej kontroly
DateTime oldTime; 
DateTime alarmTime;

Tone tone1; //premenná typu Tone - pomocou nej budeme prehrávať tóny piesne

//nasleduju premenne pre pracu playSong
int toneIndex = 0;
long int tonePauseStartTime = 0;
long int songPauseStartTime = 0;
TToneOp toneOp = TONE_OP_NONE;
//////

//deklarovanie funkcií
boolean compareDateTime(DateTime newTime, DateTime oldTime);
boolean compareTime(DateTime curTime, DateTime alarmTime);
TCmd getBtCmd();
boolean isNumeric(String s);
void showInfo();
void playSong(mySong song);
void resetSong();

void setDate(String input);
void setTime(String input);
void setAlarm(String input);
void setAlarmTime(String input);
void alarmTurnOff();
void setAlarmSong(String input);

void setup() {
  pinMode(buzzerPin, OUTPUT); //nastavenie pinu 11 ako výstupu
  Serial.begin(9600); //zahájenie sériovej komunikácie
  Serial3.begin(9600); //zahájenie sériovej komunikácie
  Wire.begin(); 
  rtc.begin();  //zahájenie merania času
  //rtc.adjust(DateTime(2018, 11, 9, 23, 48, 0));
  lcd.init(); //inicializácia LCD
  lcd.backlight();  //zapnutie podsvietenia LCD
  lcd.createChar(0,icon1);  //vytvorenie vlastného zobraziteľného znaku
  tone1.begin(buzzerPin); //zahájenie tónu
}

void loop() {
  cmd = getBtCmd();
  showInfo();
  handleCmd();
  if(isAlarmSet && alarmStarted == false) //ak je nastavený budík a zároveň ešte nezačal budenie, kontroluj čas
  {
    alarmState = compareTime(rtc.now(), alarmTime);
  }
  if(alarmState) //ak nastal čas budenia, hraj pesničku
  {
    playSong(selectedSong);
  }
}


//zobrazenie dátumu, času prípadne pozdravu na displeji
void showInfo()
{
  DateTime newTime=rtc.now(); //získanie aktuálneho času z modulu rtc

  if(!alarmState)
  {
    if(compareDateTime(newTime, oldTime)==false)
    {
      lcd.setCursor(0,0); //nastavíme ukazateľ lcd na prvý znak prvého riadku
      if(newTime.day() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.day());
      }
      else
      {
        lcd.print(newTime.day());
      }
      lcd.print(".");
      if(newTime.month() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.month());
      }
      else
      {
        lcd.print(newTime.month());  
      }
      lcd.print(".");
      lcd.print(newTime.year());
      lcd.setCursor(0,1);
      if(newTime.hour() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.hour());
      }
      else
      {
        lcd.print(newTime.hour());
      }
      lcd.print(":");
      if(newTime.minute() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.minute());
      }
      else
      {
        lcd.print(newTime.minute());
      }
      lcd.print(":");
      if(newTime.second() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.second());
      }
      else
      {
        lcd.print(newTime.second());
      }
      if(isAlarmSet)
      {
        lcd.print("  ");
        if(alarmTime.hour() < 10)
        {
          lcd.print("0");
          lcd.print(alarmTime.hour());
        }
        else
        {
          lcd.print(alarmTime.hour());
        }
        lcd.print(":");
        if(alarmTime.minute() < 10)
        {
          lcd.print("0");
          lcd.print(alarmTime.minute());
        }
        else
        {
         lcd.print(alarmTime.minute());
        }
        lcd.write(byte(0));        
      }
      else
      {
        lcd.print("        ");
      }
    }
  }
  else
  {
    if(compareDateTime(newTime, oldTime) == false)
    {
      lcd.setCursor(3,0);
      lcd.print("DOBRE RANO!");
      lcd.setCursor(4,1);
      if(newTime.hour() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.hour());
      }
      else
      {
        lcd.print(newTime.hour());
      }
      lcd.print(":");
      if(newTime.minute() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.minute());
      }
      else
      {
        lcd.print(newTime.minute());
      }
      lcd.print(":");
      if(newTime.second() < 10)
      {
        lcd.print("0");
        lcd.print(newTime.second());
      }
      else
      {
        lcd.print(newTime.second());
      }
    }
    
  }
  oldTime=newTime;
}

//získanie vstupu od používateľa zo sériovej linky 3 - je tam pripojený bluetooth
TCmd getBtCmd()
{
  String inputString = "";
  String cmdString = "";

  TCmd tempCmd = CMD_NONE;
  paramString = ""; 
  
  if(Serial3.available()>0)
  {
      inputString=Serial3.readString();
      inputString.toLowerCase();
     
      int spaceIndex = inputString.indexOf(" ");
      if(spaceIndex > 0) //ak je -1, medzera tam nie je; ak je 0(prvy znak), nezmysel
      {              
        cmdString = inputString.substring(0, spaceIndex);
        if(spaceIndex != (inputString.length()-1))
        {
          paramString = inputString.substring(spaceIndex+1, inputString.indexOf("\n"));
        }

        if(cmdString.equals("setdate"))
        {
          tempCmd = CMD_SET_DATE;
        }
        else if(cmdString.equals("settime"))
        {
          tempCmd = CMD_SET_TIME;
        }
        else if(cmdString.equals("setalarm"))
        {
          tempCmd = CMD_SET_ALARM;
        }
        else if(cmdString.equals("setalarmtime"))
        {
          tempCmd = CMD_SET_ALARM_TIME;
        }
        else if(cmdString.equals("setalarmsong"))
        {
          tempCmd = CMD_SET_ALARM_SONG;
        }
      }
      else
      {
        if(inputString.equals("alarmturnoff"))
        {
          tempCmd = CMD_ALARM_TURN_OFF;
        }
      }    
  }
  return tempCmd;
}

//zavolanie konkrétnej funkcie na základe vstupu od používateľa
void handleCmd()
{
  switch(cmd)
  {
   case CMD_NONE:
    break;
    
   case CMD_SET_DATE:
    setDate(paramString);
    break;
    
   case CMD_SET_TIME:
    setTime(paramString);
    break;
    
   case CMD_SET_ALARM:
    setAlarm(paramString);
    break;
    
   case CMD_SET_ALARM_TIME:
    setAlarmTime(paramString);
    break;
    
   case CMD_ALARM_TURN_OFF:
    alarmTurnOff();
    break;
    
   case CMD_SET_ALARM_SONG:
   setAlarmSong(paramString);
   break;
  }
}

//kontrola stringu, či sa skladá len z číslic
boolean isNumeric(String s)
{
  for(int i=0; i<s.length(); i++)
  {
    if(isdigit(s.charAt(i))==false)
    {
      return false;
    }
  }
  
  return true;
}


//kontrola, či sa dané dátumy a časy rovnajú
boolean compareDateTime(DateTime newTime, DateTime oldTime)
{
  if(newTime.day()==oldTime.day() && newTime.month()==oldTime.month() && newTime.hour()==oldTime.hour() && newTime.minute()==oldTime.minute() && newTime.second()==oldTime.second())
  {
    return true;
  }
  else
  {
    return false;
  }
}

//kontrola, či sa dané časy rovnajú
boolean compareTime(DateTime curTime, DateTime alarmTime)
{
  if(curTime.hour() == alarmTime.hour() && curTime.minute() == alarmTime.minute() && curTime.second() == alarmTime.second())
  {
    lcd.clear();
    alarmStarted = true;
    return true;
  }
  else
  {
    return false;
  }
}


//hranie piesne
void playSong(mySong song)
{
  if(toneOp == TONE_OP_NONE)
  {
     long int duration = (song.tempo*song.pTones[toneIndex].numDur)/song.pTones[toneIndex].denDur;
     tone1.play(song.pTones[toneIndex].note, duration);
     toneOp = TONE_OP_PLAYING;
  }
  else if(toneOp == TONE_OP_PLAYING)
  {
    if(!tone1.isPlaying())
    {
      tonePauseStartTime = millis();
      toneOp = TONE_OP_WAITING;
    }
  }
  else if(toneOp == TONE_OP_WAITING)
  {
    if(millis() - tonePauseStartTime >= TONEPAUSE)
    {
      toneOp = TONE_OP_NONE;
      toneIndex++;
      if(toneIndex == song.tonesCount)
      {
        songPauseStartTime = millis();
        toneOp = TONE_OP_WAITING_AFTER_SONG;
      }
    }
  }
  else if(toneOp == TONE_OP_WAITING_AFTER_SONG)
  {
      if(millis() - songPauseStartTime >= SONGPAUSE)
      { 
        toneOp= TONE_OP_NONE;
        toneIndex = 0;
      }
  }    
}

//nasledujú funkcie volané v handleCmd()
void setDate(String input)
{
  int Day = 0;
  int Month = 0;
  int Year = 0;
  
  if(isNumeric(input) && input.length() == 8)
  {
    Day = (input.substring(0,2)).toInt();
    Month = (input.substring(2,4)).toInt();
    Year = (input.substring(4,8)).toInt();
    boolean isLeap = false;
    

    if(Day < 1 || Month < 1 || Year < 2018 || Year >= 2100)
    {
      return;
    }

    if((Month == 1 || Month == 3 || Month == 5 || Month == 7 || Month == 8 || Month == 10 || Month == 12) && Day > 31)
    {
      return;
    }

    if((Month == 4 || Month == 6 || Month == 9 || Month == 11) && Day > 30)
    {
      return;
    }

    if(Year % 4 == 0)
    {
      isLeap = true;
    }

    if(Month == 2 && Day > 29 && isLeap)
    {
      return;
    }

    if(Month == 2 && Day > 28 && !isLeap)
    {
      return;
    }

    DateTime newDateTime = rtc.now();
    rtc.adjust(DateTime(Year, Month, Day, newDateTime.hour(),newDateTime.minute(),newDateTime.second()));
  }
}

void setTime(String input)
{
  int Hour = 0;
  int Min = 0;
  
  if(isNumeric(input) && input.length() == 4)
  {
    Hour = input.substring(0,2).toInt();
    Min = input.substring(2,4).toInt();

    if(Hour < 0 || Hour > 23 || Min < 0 || Min > 59)
    {
      return;
    }
    
    DateTime newDateTime = rtc.now();
    rtc.adjust(DateTime(newDateTime.year(), newDateTime.month(), newDateTime.day(), Hour, Min, 0));
  }
}
void setAlarm(String input)
{
  int tempVal = 0;
  
  if(isNumeric(input) && input.length() == 1)
  {
    tempVal = input.toInt();
    if(tempVal < 0 || tempVal > 1)
    {
      return; 
    }

    if(tempVal == 1)
    {
      isAlarmSet = true;
    }
    else
    {
      isAlarmSet = false;
    }
  }
}

void setAlarmTime(String input)
{
  int Hour = 0;
  int Min = 0;
  
  if(isNumeric(input) && input.length() == 4)
  {
    Hour = input.substring(0,2).toInt();
    Min = input.substring(2,4).toInt();

    if(Hour < 0 || Hour > 23 || Min < 0 || Min > 59)
    {
      return;
    }
    
    alarmTime = DateTime(0,0,0,Hour,Min,0);
  }
  
}

void alarmTurnOff()
{
  lcd.clear();
  alarmStarted = false;
  tone1.stop();
  alarmState == false;
}

void setAlarmSong(String input)
{
  int selection = 0;
  if(isNumeric(input) && input.length() == 1)
  {
    selection = input.toInt();
    if(selection < 1 || selection > 3)
    {
      return;
    }
    resetSong();
    switch(selection)
    {
      case 1:
        selectedSong = song1;   
        break;
      case 2:
        selectedSong = song2;
        break;
      case 3:
        selectedSong = song3;                               
        break;
    }
  }
}

//spustenie piesne od začiatku
void resetSong()
{
  toneIndex = 0;
  tonePauseStartTime = 0;
  songPauseStartTime = 0;
  toneOp = TONE_OP_NONE;
}
