/*---------------------------------------------------------------------------------------
-Owner: PHAN NGUYEN NGOC HIEN
-Date: 4-2024
-Description: This is anti theft system. The 3D model is designed by SOLIDWORKS software.
-Function: call a person if an unknown object is near the device (within set range).
-----------------------------------------------------------------------------------------*/
#include <SoftwareSerial.h>

// Variables
unsigned int distance = 0;
const uint8_t TRIG_PIN = 4;  // TRIG pin on sensor connect to pin 4 of arduino
const uint8_t ECHO_PIN = 5;  // ECO pin on sensor connect to pin 5 of arduino
const uint8_t RXD = 2;       // TXD pin on SIM900A module connect to pin 2 of arduino (it's declared as RXD pin of arduino)
const uint8_t TXD = 3;       // RXD pin on SIM900A module connect to pin 3 of arduino (it's declared as TXD pin of arduino)
// const unsigned int light = 13;                  // For test only
const uint8_t delay_in_second = 20;             // Delay time ofter call
const unsigned int sampling_time_in_ms = 500;   // Sampling time
const unsigned int alarm_range[2] = { 0, 40 };  // limit distance in cm
const unsigned int RESET_TIME_IN_SECOND = 3600;   // RESET BOARD TIME, reset after 1 hour.

//Options
bool test = true;  // For test only if test = true, else test = false
char call = 'n';   // m: message, c: call, n: nothing
// Declare CONTACTS data type
struct CONTACTS {
  char HIEN[13];
  char BE_NGHI[13];
  char BE_AN[13];
};

// Contacts
CONTACTS PHONE_NUMBER = { "+84398554576", "+84962085110", "+84338366638" };

//BAUD RATE
const unsigned int BAUD_RATE = 9600;

// DECLARE SERIAL PINS FOR SIM900A
SoftwareSerial SIM900A(RXD, TXD);

unsigned long time = millis();

// Set up
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // pinMode(light, OUTPUT);

  SIM900A.begin(BAUD_RATE);
  //SIM900A.println("ATE0");

  if (test == true) {
    Serial.begin(BAUD_RATE);
    delay(100);
    Serial.print("\nInitial DISTANCE variable: ");
    Serial.println(distance);
    Serial.println("Start....");
  }
}

// Main function
void loop() {
  // Run system
  RUN_SYSTEM(PHONE_NUMBER.HIEN);
  // Sampling time in ms
  delay(sampling_time_in_ms);

  // Calculate time to reset board
  unsigned long dt = (millis() - time) / 1000;
  if (dt == RESET_TIME_IN_SECOND) {  //second unit
    if (test == true) {              // Only print when test
      Serial.print("\nTotal run time [s]: ");
      Serial.println(dt);
      delay(100);
    }
    ResetBoard();  // Reset
  }
}

//----------------------------------------------------------------------------------------------------------
// Sub functions
void RUN_SYSTEM(char* phone_number) {
  // Calculate distance
  CALCULATE_DISTANCE(distance);
  if (test == true) {  // Only print when test
    Serial.print("\nDistance: ");
    Serial.print(distance);  // For test only
    Serial.print(" cm ");    // For test only
  }

  // Check distance
  if (distance >= alarm_range[0] and distance <= alarm_range[1]) {              // distance unit: [cm]
    if (test == true) Serial.println("...Please wait for 3s to check again!");  // Only print when test
    delay(3000);                                                                //wait 3s to check again
    
    // Calculate distance again
    CALCULATE_DISTANCE(distance);

    if (test == true) {  // Only print when test
      Serial.print("Distance: ");
      Serial.print(distance);  // For test only
      Serial.print(" cm. ");   // For test only
    }

    // Check again
    if (distance >= alarm_range[0] and distance <= alarm_range[1]) {
      if (test == true) { // Only print when test
        Serial.print("In range 0 - ");
        Serial.print(alarm_range[1]);
        Serial.print("cm. ");
        Serial.println("********Warning!!!********");
      }
      // digitalWrite(light, 1);
      switch (call) {  //there are two options to alert.

        case 'c':  //option 1, perform a call.
          if (test == true) Serial.println("Calling...");

          CALL_NUMBER(phone_number);

          if (test == true) { // Only print when test
            Serial.print("Please wait for ");
            Serial.print(delay_in_second);
            Serial.println("s to stop call.");
          }
          delay_func(delay_in_second);

          if (test == true) Serial.println("Stop call."); // Only print when test

          STOP_CALL();

          break;

        case 'm':  //option 0, send a message.
          if (test == true) Serial.println("Send message"); // Only print when test

          SEND_MESSAGE(phone_number, distance);

          delay_func(delay_in_second);

          break;

        case 'n':  //option 2, Nothing
          if (test == true) {
            Serial.println("##########################################################################");
            Serial.println("     ...........................Nothing!.............................     ");
            Serial.println("        .......................................................           ");
            Serial.println("        .......................TEST ONLY.......................           ");
            Serial.println("        .......................................................           ");
            Serial.println("     ..........................GOODBYE!!!............................     ");
            Serial.println("##########################################################################");
          }
          break;
      }
    }
  } else {
    if (test == true) {
      Serial.print("> ");
      Serial.print(alarm_range[1]);
      Serial.print("cm. ");
      Serial.print("Phone: ");
      Serial.print(phone_number);
    }
  }

  // digitalWrite(light, 0);
}
// End of void RUN_SYSTEM(char* phone_number)

void CALCULATE_DISTANCE(unsigned int& d) {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ECHO_PIN, HIGH);
  d = duration / 29 / 2;  //data range from 0 to 65535
}

void CALL_NUMBER(const char* phone_number) {
  SIM900A.print("ATD");
  SIM900A.print(phone_number);
  SIM900A.println(";");
}

void STOP_CALL() {
  SIM900A.println("ATH");
  delay(100);
}

void SEND_MESSAGE(const char* phone_number, unsigned int dist) {
  // Active SMS mode
  if (test == true) Serial.println("Active SMS mode");

  SIM900A.println("AT+CMGF=1");
  delay(500);

  // Set up phone number to send
  if (test == true) { // Only print when test
    Serial.print("Number to send: ");
    Serial.println(phone_number);
  }

  SIM900A.print("AT+CMGS=\"");
  SIM900A.print(phone_number);
  SIM900A.println("\"");
  delay(500);

  // SMS content
  SIM900A.print("Distance: ");
  SIM900A.print(dist);
  SIM900A.print("cm. Canh bao! CO TROM, vui long kiem tra.");  // The SMS text you want to send
  delay(100);

  if (test == true) Serial.print("The message has been sent. ");

  // Completed
  SIM900A.println((char)26);  // ASCII code of CTRL+Z for saying the end of sms to  the module
}
// End of void SEND_MESSAGE(const char* phone_number, unsigned int dist) {

void delay_func(unsigned int delay_in_second) {
  for (unsigned int i = 0; i < delay_in_second; i++) {
    delay(500);
  }
  for (unsigned int i = 0; i < delay_in_second; i++) {
    delay(500);
  }
}

void ResetBoard() {
  if (test == true) { // Only print when test
    Serial.println("###########################  Reset Board  ############################");
    delay(100);
  }
  asm volatile("jmp 0");
}

// End of program