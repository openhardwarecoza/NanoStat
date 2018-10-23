/* https://github.com/sparkfun/SevSeg */
#include "SevSeg.h"
/* https://github.com/0xPIT/encoder/ */
#include <ClickEncoder.h>
#include <TimerOne.h>

// Seven Segment Display
SevSeg myDisplay;
int displayType = COMMON_CATHODE; //Your display is either common cathode or common anode
//This pinout is for a regular display
//Declare what pins are connected to the digits
int digit1 = 2; //Pin 12 on my 4 digit display
int digit2 = 3; //Pin 9 on my 4 digit display
int digit3 = 4; //Pin 8 on my 4 digit display
int digit4 = 5; //Pin 6 on my 4 digit display
//Declare what pins are connected to the segments
int segA = 6; //Pin 11 on my 4 digit display
int segB = 7; //Pin 7 on my 4 digit display
int segC = 8; //Pin 4 on my 4 digit display
int segD = 9; //Pin 2 on my 4 digit display
int segE = 10; //Pin 1 on my 4 digit display
int segF = 11; //Pin 10 on my 4 digit display
int segG = 12; //Pin 5 on my 4 digit display
int segDP= 13; //Pin 3 on my 4 digit display
int numberOfDigits = 4; //Do you have a 1, 2 or 4 digit display?

// Encoder
uint8_t stepsPerNotch = 4;
ClickEncoder *encoder;
int16_t last, value;
void timerIsr() {
  encoder->service();
}

// Thermistor
#include "temperature_profile.h"
#define THERMISTOR_PIN A0
bool done = false;
#include "epcos_sensor.h"
int t = 0;

//  Solid State Relay Output and Control
const int heatPin =  A4;     // the number of the LED pin.  This also controls the heater
int heatState = HIGH;         // heatState used to set the LED and heater
long previousMillis = 0;     // will store last time LED/heater was updated
const long interval = 1000;  // interval at which to run profile checks
const long period = 200;  // interval at which to sample temperature (milliseconds)
unsigned long time_now = 0;
long time = 0;               // Time since start in seconds

// Menu
int menustate=0;

unsigned long timer;

void setup() {
  Serial.begin(9600);

  // encoder
  encoder = new ClickEncoder(A2, A1, A3, 4);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);  
  last = -1;

  // seven segment display
  myDisplay.Begin(displayType, numberOfDigits, digit1, digit2, digit3, digit4, segA, segB, segC, segD, segE, segF, segG, segDP);
  myDisplay.SetBrightness(100); //Set the display to 100% brightness level

  timer = millis();
}

void loop() {

  // Update current temp once per interval
  if(millis() > time_now + period){
    time_now = millis();
    t = temperature();
  }

  // Get EncoderValue
  value += encoder->getValue(); 
  if (value != last) {
    timer = millis();
    if (menustate == 0) {
      if (value > 2) {value = 2;};
      if (value < 0) {value = 0;};        
    } else if (menustate == 1) {
      if (value > 3) {value = 3;};
      if (value < 1) {value = 1;};        
    } else if (menustate == 1) {
      if (value > 150) {value = 150;};
      if (value < 30) {value = 30;};        
    }
    last = value;
  }

  // Get EncoderButton
  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    timer = millis();
    switch (b) {
      // ClickEncoder::Pressed
      // ClickEncoder::Held
      // ClickEncoder::Released
      // ClickEncoder::Clicked
      case ClickEncoder::DoubleClicked:
          Serial.println("ClickEncoder::DoubleClicked");
      case ClickEncoder::Held:
          // Holding cancels the job
          Serial.println("ClickEncoder::Held");
          menustate = 0;
          value = 0;
      case ClickEncoder::Clicked:
          Serial.print("Menu: ");
          Serial.print(menustate);
          Serial.print(" / value = ");
          Serial.println(value);
          if (menustate == 0) {
            if (value == 1) {
              Serial.println("Entering Reflow");
              menustate = 1;
              value = 0;
            } else if (value == 2) {
              Serial.println("Entering Dry");
              menustate = 2;
              value = 0;
            };
          };
          if (menustate == 1) {
            if (value == 1) {
              Serial.println("Reflow Leaded");
              // menustate = 1;
            } else if (value == 2) {
              Serial.println("Reflow Leadfree");
              // menustate = 2;
            } else if (value == 3) {
              Serial.println("Going back!");
              menustate = 0;
              value = 0;
            };
          };
          if (menustate == 2) {
            Serial.print("Entering Drying Cycle: Running at ");
            Serial.println(value);
          };
      break;
    }
  } // end ifbutton    

  // IDLE and Active Display
  
  if (millis() - timer >= 2000) {
    // IDLE:
    char tempString[10]; //Used for sprintf
    sprintf(tempString, "%4d", t); //Convert value into a string that is right adjusted
    myDisplay.DisplayString(tempString, 8); 
    // myDisplay.DisplayString("IDLE", 0); 
  } else {
    // Active:
    if (menustate == 0) { // MAIN MENU
      if (value == 0 ){
        myDisplay.DisplayString("OFF ", 0); 
      } else if (value == 1 ){
        myDisplay.DisplayString("FLO ", 0); 
      } else if (value == 2 ){
        myDisplay.DisplayString("DRY ", 0); 
      };   
    };
      if (menustate == 1) { // REFLOW PROFILE SELECTION MENU
      if (value == 1 ){
        myDisplay.DisplayString("PBSN", 0);
      } else if (value == 2 ){
        myDisplay.DisplayString("PBFR", 0); 
      } else if (value == 3 ){
        myDisplay.DisplayString("BAC ", 0); 
      };   
    };
      if (menustate == 2) { // DRYING TEMP SELECTION
      char tempString[10]; //Used for sprintf
      sprintf(tempString, "%4d", value); //Convert value into a string that is right adjusted
      myDisplay.DisplayString(tempString, 0); 
    };  
  };
  delay(1); 
}
