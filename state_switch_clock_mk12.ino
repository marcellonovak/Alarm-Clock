/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <pitches.h>

#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#include "RTClib.h"
RTC_DS1307 rtc;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int leftButton = 2; // Interrupt Pin 0 -- TOP
const int rightButton = 3; // Interrupt Pin 1 -- 2nd
const int displayStateButton = 18; // Interrupt Pin 5 -- 3rd
const int alarmButton = 19; // Interrupt Pin 4 -- BOTTOM

int redPin = 4; 
int greenPin = 5; // RGB LED Pins
int bluePin = 6;

int buzzerPin = 22;
int alarmPin = 13; // Alarm LED Pin

bool clearScreen = false;

bool wasLeftButtonPressed = false;
bool wasRightButtonPressed = false;
bool alreadyRang = false;

// Display state machine support
enum DeviceDisplayState {CLOCK, ALARM, DATE, YEAR}; // All different states
DeviceDisplayState displayState = CLOCK; // Initially in Clock State

// Alarm state machine support
enum DeviceAlarmState {ARMED_INACTIVE, ARMED_ACTIVE, DISARMED};
DeviceAlarmState alarmState = DISARMED;

uint16_t alarmTime = 7 * 60; // Number of minutes since midnight
uint16_t resetTime = alarmTime + 1; // Reset in 15 seconds

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  lcd.begin(16, 2);
  Serial.begin(57600);
    
  // Set the time:: //
  
  const uint8_t hourInit = 6;
  const uint8_t minuteInit = 59;
  const uint8_t secondInit = 55;
  
  const uint8_t dayInit = 12;
  const uint8_t monthInit = 5;
  const uint16_t yearInit = 2020;

  if (!rtc.begin())
  {
    Serial.println("RTC is busted");
  }
  rtc.adjust(DateTime(yearInit, monthInit, dayInit, hourInit , minuteInit, secondInit));
  
  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  pinMode(displayStateButton, INPUT_PULLUP);
  pinMode(alarmButton, INPUT_PULLUP);

  attachInterrupt(0, leftButtonPressed, FALLING); 
  attachInterrupt(1, rightButtonPressed, FALLING); 
  attachInterrupt(5, switchToNextDisplayState, FALLING); 
  attachInterrupt(4, alarmStateButtonPressed, FALLING);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(alarmPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  switchToClockState();
};
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RGB_color(int red_light_value, int green_light_value, int blue_light_value) {
  analogWrite(redPin, red_light_value);
  analogWrite(greenPin, green_light_value);
  analogWrite(bluePin, blue_light_value);
}

void leftButtonPressed() {
  if (displayState == ALARM) {
    if (alarmTime > 0)
    {
      alarmTime++;
    }
  }
  else
  {
    wasLeftButtonPressed = true;
  }
}

void rightButtonPressed() {
  if (displayState == ALARM)
  {
    if (alarmTime < 24 * 60)
    {
      alarmTime--;
    }
  }
  else {
    wasRightButtonPressed = true;
  }
}

void alarmStateButtonPressed()
{
  switch(alarmState) {
    case ARMED_INACTIVE:
      alarmState = DISARMED;
      break;
        
    case DISARMED:
      alarmState = ARMED_INACTIVE;
      break;

    case ARMED_ACTIVE:
      alarmState = ARMED_INACTIVE;
      break;
  }
  
  // Alarm LED is on in all states except "disarmed"
  digitalWrite(alarmPin, (alarmState != DISARMED) ? HIGH : LOW);
}

void switchToClockState() {
  displayState = CLOCK;
  RGB_color(255, 0, 0);
}

void switchToAlarmState() {
  displayState = ALARM;
  RGB_color(255, 125, 0);
}

void switchToDateState() {
  displayState = DATE;
  RGB_color(0, 255, 0);
}

void switchToYearState() {
  displayState = YEAR;
  RGB_color(0, 0, 255);
}


void switchToNextDisplayState() {
  clearScreen = true;
  switch (displayState) {
    
    case CLOCK:
      switchToAlarmState();
      break;
      
    case ALARM:
      switchToDateState();
      break;
      
    case DATE:
      switchToYearState();
      break;
      
    case YEAR:
      switchToClockState();   
      break;
  }
}

String WithLeadingZeros(uint16_t number) {
  if (number < 10) {
    return "0" + String(number);
  }
  else {
    return String(number);
  }
}

void PlayNote(unsigned int frequency, float toneDuration, float delayDuration) {
  tone(buzzerPin, frequency, toneDuration);
  delay(delayDuration);
}

bool IsLeapYear(unsigned int year)
{
  if (year % 400 == 0) return true;
  if (year % 100 == 0) return false;
  if (year % 4 == 0) return true;
  return false;
}

int32_t NumberOfSecondsInAYear(int32_t year)
{
  int32_t NumberOfDaysInAYear = 365 + (IsLeapYear(year) ? 1 : 0);
  return NumberOfDaysInAYear * 24 * 3600;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  struct AlarmNote {
    unsigned int frequency;
    float toneDuration;
    float delayDuration;
  };

  static const AlarmNote Measure1[] = {{NOTE_CS5, 666.67, 666.67},
                                       {NOTE_FS5, 444.45, 222.225},
                                       {NOTE_F5, 444.45, 222.225},
                                       {NOTE_DS5, 444.45, 222.225},
                                       {NOTE_CS5, 666.67, 666.67},
                                       {NOTE_AS4, 666.67, 666.67}};

  static const AlarmNote Measure2[] = {{NOTE_AS4, 444.45, 222.225},
                                       {NOTE_B4, 444.45, 222.225},
                                       {NOTE_CS5, 444.45, 222.225},
                                       {NOTE_GS4, 444.45, 222.225},
                                       {NOTE_AS4, 444.45, 222.225},
                                       {NOTE_B4, 444.45, 222.225},
                                       {NOTE_AS4, 666.67, 666.67},
                                       {NOTE_CS5, 666.67, 666.9}};

  static const AlarmNote Measure3[] = {{NOTE_CS5, 666.67, 666.67},
                                       {NOTE_FS5, 444.45, 222.225},
                                       {NOTE_F5, 444.45, 222.225},
                                       {NOTE_DS5, 444.45, 222.225},
                                       {NOTE_CS5, 666.67, 666.67},
                                       {NOTE_FS5, 666.67, 666.67}};

  static const AlarmNote Measure4[] = {{NOTE_FS5, 444.45, 222.225},
                                       {NOTE_GS5, 444.45, 222.225},
                                       {NOTE_FS5, 444.45, 222.225},
                                       {NOTE_F5, 444.45, 222.225},
                                       {NOTE_DS5, 444.45, 222.225},
                                       {NOTE_F5, 444.45, 222.225},
                                       {NOTE_FS5, 666.67, 666.67}};
  
  DateTime currentTime = rtc.now();
  DateTime newTime = currentTime;
  String firstRow = "Idiot Robot Mk12", secondRow;

  if (clearScreen) {
    lcd.clear();
    clearScreen = false;
  }

  if ((alarmTime == currentTime.hour() * 60 + currentTime.minute()) && (alarmState == ARMED_INACTIVE)) {
    alarmState = ARMED_ACTIVE;
  }

  if ((resetTime == currentTime.hour() * 60 + currentTime.minute()) && (alarmState == ARMED_ACTIVE)) {
    alarmState = ARMED_INACTIVE;
    alreadyRang = false;
  }

  switch (displayState) {
    case CLOCK: {
      if (wasLeftButtonPressed) {
        if (currentTime.hour() < 23) {
          TimeSpan ts(3600);
          newTime = currentTime + ts;
        }
        else { // do not roll over the day by upping the hour, go back to zero hours
          TimeSpan ts(3600);
          for (int i = 0; i < 23; i++) {
            newTime = currentTime - ts;        
          }
        }
        rtc.adjust(newTime);
        wasLeftButtonPressed = false;
      }
      else if (wasRightButtonPressed) {
        if (currentTime.minute() < 59) {
          TimeSpan ts(60);
          newTime = currentTime + ts;
        }
        else { // Don't roll over the minutes into the hours
          TimeSpan ts(60 * 59);
          newTime = currentTime - ts;
        }
        rtc.adjust(newTime);
        wasRightButtonPressed = false;
      }
      
      secondRow = "Time> " + WithLeadingZeros(newTime.hour()) + ":" + WithLeadingZeros(newTime.minute()) + ":" + WithLeadingZeros(newTime.second());
      }
      break;
      
    case ALARM: {
        uint16_t alarmHours = alarmTime / 60;
        uint16_t alarmMinutes = alarmTime - 60 * alarmHours;
        secondRow = "Alarm> " + WithLeadingZeros(alarmHours) + ":" + WithLeadingZeros(alarmMinutes);
      }
      break;
      
    case DATE: {
      TimeSpan ts(3600);
      if (wasLeftButtonPressed) {
        for (int i = 0; i < 24; i++) {
          newTime = newTime + ts;        
        }
        wasLeftButtonPressed = false;
      }
      else if (wasRightButtonPressed) {
        for (int i = 0; i < 24; i++) {
          newTime = newTime - ts;
        }
        wasRightButtonPressed = false;
      }
         
      rtc.adjust(newTime);
      secondRow = "Date> " + WithLeadingZeros(newTime.month()) + " - " + WithLeadingZeros(newTime.day());
      }
      break;

    case YEAR: {
      TimeSpan ts(NumberOfSecondsInAYear(currentTime.year()));
      if (wasLeftButtonPressed) {
        newTime = currentTime + ts;
        wasLeftButtonPressed = false;
        rtc.adjust(newTime);
      }
      
      else if (wasRightButtonPressed) {
        newTime = currentTime - ts;
        wasRightButtonPressed = false;
        rtc.adjust(newTime);
      }

      secondRow = "Year> " + String(newTime.year());
      }
      break;
  }

  switch (alarmState) {
    case ARMED_ACTIVE: {
      if (alreadyRang == false) {
        int i;
        
        // Refresh the screen
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("This dumb jingle");
        lcd.setCursor(0, 1);
        lcd.print("took too long");
  
        // Measure 1
        for (i = 0; (alarmState == ARMED_ACTIVE) && (i < sizeof(Measure1) / sizeof(Measure1[0])); i++)
        {
          PlayNote(Measure1[i].frequency, Measure1[i].toneDuration, Measure1[i].delayDuration);
        }
  
        // Measure 2
        for (i = 0; (alarmState == ARMED_ACTIVE) && (i < sizeof(Measure2) / sizeof(Measure2[0])); i++) 
        {
          PlayNote(Measure2[i].frequency, Measure2[i].toneDuration, Measure2[i].delayDuration);
        }
  
        // Refresh the screen
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Quarantine Sucks");
        lcd.setCursor(0, 1);
        lcd.print("I'm going nuts");
  
        // Measure 3
        for (i = 0; (alarmState == ARMED_ACTIVE) && (i < sizeof(Measure3) / sizeof(Measure3[0])); i++)
        {
          PlayNote(Measure3[i].frequency, Measure3[i].toneDuration, Measure3[i].delayDuration);
        }
  
        // Measure 4
        for (i = 0; (alarmState == ARMED_ACTIVE) && (i < sizeof(Measure4) / sizeof(Measure4[0])); i++)
        {
          PlayNote(Measure4[i].frequency, Measure4[i].toneDuration, Measure4[i].delayDuration);
        }
  
        // Whole rest
        delay(2666.67);
        
        alarmState = ARMED_INACTIVE;
        alreadyRang = true;
        Serial.print("Already Rang");
        Serial.println();
        lcd.clear();
      }
      break;         
    }
  }
    
  lcd.setCursor(0, 0);
  lcd.print(firstRow);
  lcd.setCursor(0, 1);
  lcd.print(secondRow);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
