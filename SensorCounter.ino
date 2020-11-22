/*  SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)
*/

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

#define SAMPLING  60000 /* in miliseconds */
#define DEBOUNCE  200 /* devounce time for a buttons */
#define SENSOR_NUMBER 1


File myFile;
const int rs = 9, en = 10, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


unsigned char sd_slot = 4;
unsigned char waterSensor = 2;
unsigned char switch_button = 3;

/* longs for time looping */
unsigned long currentTime;
unsigned long c_loopTime;

/* ints for interupts  */
volatile int flow_frequency = 0; /* interupt value for waterSensor */
volatile int electric_frequency = 0; /* interupt value for waterSensor */
volatile int switch_state = 0; /* interupt value for cycling trough display modes */
unsigned int old_switch_state = -1;
unsigned int water_min;
unsigned int elect_min;
int old_water_min = -1;
int old_elect_min = -1;
String dataString = ""; /* String for saving to CSV  */


void water_usage(); /* function for showing water usage on LCD */
void test_usage(); /* test for switching display valuses */
void flow (); /* interupt for waterSensor */
void screen_switch(); /* interupt, cycling trough display modes */


void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done!\n");
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("File opened!");
  }
  else {
    Serial.println("Failed to open file!");
    while (1) {
      ;
    }
  }

  pinMode(waterSensor, INPUT);
  digitalWrite(waterSensor, HIGH);
  attachInterrupt(0, flow, RISING);

  pinMode(switch_button, INPUT);
  digitalWrite(switch_button, HIGH);
  attachInterrupt(1, screen_switch, HIGH);
  sei();
  currentTime = millis();
  c_loopTime = currentTime;
}

void loop() {
  currentTime = millis();
  /* loop for waterSensor */
  if (currentTime >= (c_loopTime + SAMPLING)) {
    c_loopTime = currentTime;

    water_min = flow_frequency * 10;
    flow_frequency = 0;

    elect_min = electric_frequency * 10;
    electric_frequency = 0;

    for (int j = 0; j < SENSOR_NUMBER; j++) {
      dataString = String(water_min) + "," + String(elect_min);
      myFile.println(dataString);
      Serial.println(dataString);
      myFile.flush();
      dataString = "";
    }
  }

  switch (switch_state) {
    case 0:
      if (water_min != old_water_min || switch_state != old_switch_state) {
        water_usage();
        old_water_min = water_min;
        old_switch_state = switch_state;
      }
      break;

    case 1:
      if (elect_min != old_elect_min || switch_state != old_switch_state) {
        test_usage();
        old_elect_min = elect_min;
        old_switch_state = switch_state;
      }
      break;

  }

}
void flow () {
  flow_frequency++;
  delay(DEBOUNCE);
}


void screen_switch() {
  switch_state++;
  switch_state = switch_state % 2;
  delay(DEBOUNCE);
}


void water_usage() {
  lcd.clear();
  lcd.print("Water usage:");
  lcd.setCursor(0, 1);
  lcd.print(String(water_min)+ " l/min");
  Serial.println(water_min);
}


void test_usage() {
  lcd.clear();
  lcd.print("Test_value");
  lcd.setCursor(0, 1);
  lcd.print(String(elect_min) + " W/m");
}
