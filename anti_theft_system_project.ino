/*---------------------------------------------------------------------------------------
-Owner: PHAN NGUYEN NGOC HIEN
-Date: 4-2024
-Description: This is anti theft system. The 3D model is designed by SOLIDWORKS software.
-Function: call a person if an unknown object is near the device (within set range).
-----------------------------------------------------------------------------------------*/
#include <SoftwareSerial.h>

// Variables
unsigned int distance = 0;
const uint8_t TRIG_PIN = 4;                      // TRIG pin on sensor connect to pin 4 of arduino
const uint8_t ECHO_PIN = 5;                      // ECO pin on sensor connect to pin 5 of arduino
const uint8_t RXD = 8;                           // TXD pin on SIM900A module connect to pin 2 of arduino (it's declared as RXD pin of arduino)
const uint8_t TXD = 9;                           // RXD pin on SIM900A module connect to pin 3 of arduino (it's declared as TXD pin of arduino)
const unsigned int light = 13;                   // For TEST only
const uint8_t delay_in_second = 20;              // Delay time ofter call
const unsigned int sampling_time_in_ms = 500;    // Sampling time
unsigned int alarm_range[2] = { 0, 40 };         // limit distance in cm
const unsigned int RESET_TIME_IN_SECOND = 3600;  // RESET BOARD TIME, reset after 1 hour.
const unsigned int MAX_DISTANCE = 200;  //Only use distance up to 300 cm
unsigned int A7_value = 0;
int analogPin = A7;
//Options
const bool TEST = false;  // For TEST only if TEST = true, else TEST = false

// Declare CONTACTS data type
struct CONTACTS 
{
  char HIEN[13];     // 1
  char CAU_HIEN[13];  // 2
};

//Contacts
CONTACTS PHONE_NUMBER = { "+84398554576", "+84975568882"};

//BAUD RATE
const unsigned int BAUD_RATE = 9600;

// DECLARE SERIAL PINS FOR SIM900A
SoftwareSerial SIM900A(RXD, TXD);

unsigned long time = millis();

// Set up
void setup() 
{
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(light, OUTPUT);
  
  SIM900A.begin(BAUD_RATE);

  if (TEST == true) {
    Serial.begin(BAUD_RATE);
    delay(100);
    Serial.print("\nInitial DISTANCE variable: ");
    Serial.println(distance);
    Serial.println("Start....");
  }
  digitalWrite(light, 0);
}

// Main function
void loop() 
{
  //Set distance
  A7_value = analogRead(analogPin);
  alarm_range[1] = map(A7_value, 0, 1023, 0, MAX_DISTANCE);
  if(1010 < A7_value & A7_value <= 1023) alarm_range[1] = MAX_DISTANCE;
  
  // Run system
  RUN_SYSTEM(PHONE_NUMBER);

  // Sampling time in ms
  delay(sampling_time_in_ms);

  // Calculate time to reset board
  unsigned long dt = (millis() - time) / 1000;
  if (dt >= RESET_TIME_IN_SECOND) {  //second unit
    if (TEST == true) {              // Only print when TEST
      Serial.print("\nRESET BOARD,   Total run time [s]: ");
      Serial.println(dt);
      delay(100);
    }
    ResetBoard();  // Reset
  }
}

//----------------------------------------------------------------------------------------------------------
// Sub functions
void RUN_SYSTEM(CONTACTS phone_number) 
{
  // Turn off light
  if(TEST == true) digitalWrite(light, 0);

  // Calculate distance
  CALCULATE_DISTANCE(distance);

  if (TEST == true) 
  {  // Only print when TEST
    Serial.print("\nd ");
    Serial.print(distance);  
    Serial.print(" cm ");
  }

  // Check again after 2s
  if (distance >= alarm_range[0] && distance <= alarm_range[1]) // distance unit: [cm]
  { 
    // Delay 500ms then check again to avoid value noise
    delay(500);
    // re-calculate distance
    CALCULATE_DISTANCE(distance);

    if (distance >= alarm_range[0] && distance <= alarm_range[1]){ 
      if (TEST == true) 
      {                                             // Only print when TEST
        Serial.print(". In range 0 - ");
        Serial.print(alarm_range[1]);
        Serial.println("cm. ");
        Serial.println("******** a stranger ********");  //Detect stranger
        digitalWrite(light, 1);
      }
      CALL_PERSON(PHONE_NUMBER, TEST);
    }
  }
  else 
  {
    if (TEST == true) 
    {
      Serial.print("> max ");
      Serial.print(alarm_range[1]);
      Serial.print(" cm. ");
      Serial.print("Using: ");
      Serial.print(phone_number.HIEN);
    }
  }
}
// End of void RUN_SYSTEM()

void CALCULATE_DISTANCE(unsigned int& d) {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ECHO_PIN, HIGH);
  d = duration / 29 / 2;  //data range from 0 to 65535
}

void CALL_PERSON(CONTACTS phone_number, bool TEST)
{
  //The third person
      if (TEST==true) 
      {
        Serial.print("Call ");
        Serial.println(phone_number.HIEN);

        CALL_NUMBER(phone_number.HIEN);

        Serial.print("Automatically stop calls after ");
        Serial.print(delay_in_second);
        Serial.println("s.");

        delay_func(delay_in_second);
        
        Serial.println("Stop call.");
        STOP_CALL();
        delay(100);
      } 
      else 
      {  // if TEST == false and don't print anything
        CALL_NUMBER(phone_number.CAU_HIEN);
        delay_func(delay_in_second);
        STOP_CALL();
        delay(100);
      }
   
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
  if (TEST == true) {
    Serial.println("Active SMS mode");
  }

  SIM900A.println("AT+CMGF=1");
  delay(500);

  // Set up phone number to send
  if (TEST == true) {  // Only print when TEST
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

  if (TEST == true) Serial.print("The message has been sent. ");

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
  if (TEST == true) {  // Only print when TEST
    Serial.println("        #######################################################       ");
    Serial.println("###########################  Reset Board  ############################");
    Serial.println("        #######################################################       ");
    delay(100);
  }
  asm volatile("jmp 0");
}


// End of program