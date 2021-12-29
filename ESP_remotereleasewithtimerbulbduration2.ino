//This code is in the Public Domain (or CC0 licensed, at your option.)
//By Analogue Photography Talk - Dec 2021
// Youtube channel https://www.youtube.com/channel/UCxGzTCecw_vtLpeFYcNhSOw
//Website : www.sgwetplate.com

//This is coded for a ESP32 Dev Module for the Remote Mechanical cable release unit.
//It uses the classic Bluetooth to communicate with the mobile app


#include "BluetoothSerial.h"
#include <ESP32Servo.h>


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

 
int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 32;
int ONBOARD_LED = 2;
char incomingChar;
String message; //whatBT appsend over
int bulb_counter=0;  //track how many time BULB is sent over via BT 
int timer_counter=0; //track how many time timer is sent over via BT sp that we can cancel the timer if it is sent the 2nd time
int bulb_timer_counter=0; //track how many time BULB timer is sent over via BT sp that we can cancel the BULB timer if it is sent the 2nd time
unsigned long StartTime;
unsigned long CurrentTime;
unsigned long ElapseTime;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("APTRemote"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
// Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 500, 2400); // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
  myservo.write(180);
    pinMode(ONBOARD_LED,OUTPUT);
}

//fuction to press n release immediately
void trigger(){
for (pos = 180; pos >= 0; pos -= 10) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(25);             // waits 15ms for the servo to reach the position
     
  }
  Serial.println("0 reached");
 
for (pos = 0; pos <= 180; pos += 20) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(25);             // waits 15ms for the servo to reach the position
    
  }
  Serial.println("180 reached");
   message = "";
   bulb_counter=0; //incase on was press during Bulb mode
}

//fuction to press only
void trigger_ON(){
for (pos = 180; pos >= 0; pos -= 10) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(25);             // waits 15ms for the servo to reach the position
     
  }
  Serial.println("0 reached");
 
   message = "";
 //  bulb_counter=0; //in case on was press during Bulb mode, no need this now as ON is now disabled in timer mode
}

//fuction to release only
void trigger_OFF(){
for (pos = 0; pos <= 180; pos += 20) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(25);             // waits 15ms for the servo to reach the position
    
  }
  Serial.println("180 reached");
   message = "";
  // bulb_counter=0; //in case on was press during Bulb mode , no need this now as ON is now disabled in timer mode
}



void loop() {
 
//read in BT message 
while(SerialBT.available()){
 // Serial.print("got data");
 char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{
      message = "";
    }
   // org_message=message;
    Serial.write(incomingChar); 
}

if (message.length()>0){
Serial.print("BT message :");
Serial.println(message);
}

if (message=="bulb"){
  if (bulb_counter==0){ 
  trigger_ON();
     
  
  Serial.println("BULB mode reached");
  message = "";
  bulb_counter=1;
  }
  else
  { 
 trigger_OFF();
  Serial.println("BULB mode ended");
  message = "";
  bulb_counter=0;
  }
  
}
else if (message=="on"){
trigger();
  
}
else if (message=="timer"){
timer_counter=timer_counter+1;    //anything more than 1 means timer button send a 2nd time so to cancel timer
Serial.print("timer counter ;");
Serial.println(timer_counter);
Serial.println(SerialBT.available());
 if (timer_counter==1){
  StartTime=millis();  
  Serial.print("StartTime :");
Serial.println(StartTime);
 }

if(ElapseTime<7000 && SerialBT.available()==false){ //blink led if within 5 secs  & no cancel timer

  delay(1000);
  digitalWrite(ONBOARD_LED,HIGH);
  delay(100);
  digitalWrite(ONBOARD_LED,LOW);
  ElapseTime=millis()-StartTime;
  Serial.print("Elapse time :");
  Serial.println(ElapseTime);
}

else if (ElapseTime<10000 && SerialBT.available()==false){  //blink led FASTER if more than 7 but less than 10 secs

  delay(300);
  digitalWrite(ONBOARD_LED,HIGH);
  delay(100);
  digitalWrite(ONBOARD_LED,LOW);
  ElapseTime=millis()-StartTime;
  Serial.print("Elapse time :");
  Serial.println(ElapseTime);
}
else if (ElapseTime>10000) 
 {trigger();
  Serial.print("Timer Delay reached");
  message="";
  ElapseTime=0;  
  timer_counter=0;
}
if (SerialBT.available()==true)
{
  
  Serial.println("came into the SerialBT FLUSH!!");
  SerialBT.flush();
  Serial.flush();
  Serial.println("FLUSH!!!");
  ElapseTime=0;  
  timer_counter=0;
  message="";}

}
else if (message.substring(0,5)=="zbulb"){
  /*   Serial.print("message is");
  Serial.println("time bulb detected");*/
  bulb_timer_counter=bulb_timer_counter+1;  
  Serial.println(bulb_timer_counter);
  String temp_str=message.substring(5);
 /*  Serial.print("zzzzBulb time string :");
       Serial.println(temp_str); */
   //delay(1000);    
  int bulb_duration =temp_str.toInt();
   trigger_ON();
     Serial.print("Bulb time :");
       Serial.println(bulb_duration);
 // for (int i=0; i <= bulb_duration; i++){
   //  delay(1000);
   //}

 if (bulb_timer_counter==1){
  StartTime=millis();  
 /* Serial.print("StartTime :");
Serial.println(StartTime); */
 }
  
while (ElapseTime<(bulb_duration*1000)) {  //blink led if within 5 secs

  delay(1000);
  ElapseTime=millis()-StartTime;
  /*Serial.print("Elapse time :");
  Serial.println(ElapseTime);
  Serial.print("message in this loop");
  Serial.println(message);
   Serial.print("serial BT has data? :");
  Serial.println(SerialBT.available()); */
  if (SerialBT.available()>0)
  {
   Serial.println("came into the SerialBT FLUSH!!");
  SerialBT.flush();
  Serial.flush();
  Serial.println("cancel bulb timer!!!");
  ElapseTime=0;  
  timer_counter=0;
   trigger_OFF();
  message="xxx";
   break; }
}

 if (ElapseTime>=(bulb_duration*1000) ){
    trigger_OFF();
     ElapseTime=0;  
   message="";
 }
}

else 
  {//delay(1000);
  //Serial.println("came into the final else!!");
  message="";
  ElapseTime=0;
  StartTime=0;
  timer_counter=0;
  bulb_timer_counter=0;}
}//end of loop
