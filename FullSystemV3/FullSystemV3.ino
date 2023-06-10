#include<Wire.h>
#include <Servo.h>

const int MPU_addr=0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;
int16_t oldAcX,oldAcY,oldAcZ;
int minVal=265;
int maxVal=402;

double x;
double y;
double z;

#define trigPin 2
#define echoPin 3
long duration;
int distance;

#define trigPin2 4
#define echoPin2 5
long duration2;
int distance2;

#define trigPin3 6
#define echoPin3 7
long duration3;
int distance3;

#define trigPin4 10
#define echoPin4 11
long duration4;
int distance4;

#define FALSE_ALARM_BUTTON 47
#define GET_LOCATION_BUTTON 49
#define VOICE_SWITCH 46
#define HAPTIC_SWITCH 48
#define CV_SWITCH 50

#define buzzer 37

Servo servo;
Servo servo2;

String accelTrig;
String falseAlarmTrig;
String getLocTrig;
String voiceTrig;
String hapticTrig;
String cvTrig;
String obstacleDetectMataasTrig;
String obstacleDetectMababaTrig;
String obstacleDetectKaliwaTrig;
String obstacleDetectKananTrig;


int falseAlarmState = digitalRead(FALSE_ALARM_BUTTON);
int getLocState = digitalRead(GET_LOCATION_BUTTON);
int voiceState = digitalRead(VOICE_SWITCH);
int hapticState = digitalRead(HAPTIC_SWITCH);
int cvState = digitalRead(CV_SWITCH);

int sensorEqualizer = 0;

unsigned long previousMillis = 0;
const long period = 20000;

boolean emergencyCountdown = false;
boolean getPrevMillis = true;
boolean falseAlarmButtonPushed = true;

void setup() {

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  AcX = 0;
  AcY = 0;
  AcZ = 0;
  oldAcX = 0;
  oldAcY = 0;
  oldAcZ = 0;

  servo.attach(12);
  servo2.attach(13);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);
  
  pinMode(FALSE_ALARM_BUTTON, INPUT_PULLUP);
  pinMode(GET_LOCATION_BUTTON, INPUT_PULLUP);
  pinMode(VOICE_SWITCH, INPUT_PULLUP);
  pinMode(HAPTIC_SWITCH, INPUT_PULLUP);
  pinMode(CV_SWITCH, INPUT_PULLUP);

  Serial.begin(9600);
  Serial1.begin(9600);


}

void loop() {

  MPUGyro();
  recordGyroRegisters();

  falseAlarmState = digitalRead(FALSE_ALARM_BUTTON);
  getLocState = digitalRead(GET_LOCATION_BUTTON);
  voiceState = digitalRead(VOICE_SWITCH);
  hapticState = digitalRead(HAPTIC_SWITCH);
  cvState = digitalRead(CV_SWITCH);

  //SERVO ALIGN===========================================================
  if(x >= 0 && x <= 90){
    int servoVal = map(x, 0, 90, 0, 90);
    sensorEqualizer = map(x, 0, 90, 65, 0);
    servo.write(servoVal);
    servo2.write(servoVal);
    Serial.print("SERVO VAL: ");
    Serial.println(servoVal);
    Serial.print("EQUALIZER VAL: ");
    Serial.println(sensorEqualizer);    

    
    //LAGAY KA CODE HERE THAT ADDS A CERTAIN AMOUNT TO DISTANCE2 BASED SA SERVOVAL
    
  }

  distanceSens();
  distanceSens2();
  distanceSens3();
  distanceSens4();

  ultrasonicDetect();    


  //FALSE ALARM BUTTON==========================================================
  if(falseAlarmState == 0){
    falseAlarmTrig = "ON";
  }  
  else{
    falseAlarmTrig = "OFF";
  }

  //GET LOCATION BUTTON==========================================================
  if(getLocState == 0){
    getLocTrig = "ON";
  }  
  else{
    getLocTrig = "OFF";
  }

  //VOICE SWITCH==========================================================
  if(voiceState == 0){
    voiceTrig = "ON";
  }  
  else{
    voiceTrig = "OFF";
  }

  //HAPTIC SWITCH=========================================================
  if(hapticState == 0){
    hapticTrig = "ON";
  }  
  else{
    hapticTrig = "OFF";
  }

  //CV SWITCH=============================================================
  if(cvState == 0){
    cvTrig = "ON";
  }  
  else{
    cvTrig = "OFF";
  }

  //FALL DETECT=====================================================================
  if(getPrevMillis == true){
    previousMillis = millis();
  }

  if(abs(rotX) >= 250.13 || abs(rotY) >= 250.13 || abs(rotZ) >= 250.13){
    emergencyCountdown = true;
    getPrevMillis = false;
    falseAlarmButtonPushed = false;
    tone(buzzer, 500);
  }  

  if(emergencyCountdown == true){

    unsigned long currentMillis = millis(); // store the current time
    if (currentMillis - previousMillis >= period) { // check if 1000ms passed

      if(falseAlarmButtonPushed == false){
        accelTrig = "ON";
        falseAlarmButtonPushed = true;
        emergencyCountdown = false;
        getPrevMillis = true;
        noTone(buzzer);
        delay(3000);
      }

      else{
        emergencyCountdown = false;
        getPrevMillis = true;     
        noTone(buzzer);
      }

    }    

  }  

  //FALSE ALARM BUTTON===========================================================
  if(falseAlarmState == 0){
    falseAlarmButtonPushed = true;
    noTone(buzzer);
  }  


  //voiceTrig;hapticTrig;cvTrig;getLocationTrig;accelTrig;mataas;mababa;kaliwa;kanan
  //Serial1.print(falseAlarmTrig+";"+getLocTrig+";"+hapticTrig+";"+voiceTrig+";"+cvTrig);
  Serial1.print(voiceTrig+";"+hapticTrig+";"+cvTrig+";"+getLocTrig+";"+accelTrig+";"+obstacleDetectMataasTrig+";"+obstacleDetectMababaTrig+";"+obstacleDetectKaliwaTrig+";"+obstacleDetectKananTrig+";");

  Serial.println(falseAlarmTrig+";"+
                  getLocTrig+";"+
                  hapticTrig+";"+
                  voiceTrig+";"+
                  cvTrig+";"+falseAlarmButtonPushed+falseAlarmState);

  delay(400); 

  accelTrig = "OFF";
  obstacleDetectMataasTrig = "OFF";
  obstacleDetectMababaTrig = "OFF";
  obstacleDetectKaliwaTrig = "OFF";
  obstacleDetectKananTrig = "OFF";  

}


void MPUGyro()  {

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);
  AcX=Wire.read()<<8|Wire.read();
  AcY=Wire.read()<<8|Wire.read();
  AcZ=Wire.read()<<8|Wire.read();
  int xAng = map(AcX,minVal,maxVal,-90,90);
  int yAng = map(AcY,minVal,maxVal,-90,90);
  int zAng = map(AcZ,minVal,maxVal,-90,90);
  
  x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
  y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
  z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);

  Serial.print("AngleX= ");
  Serial.println(x);
  
  Serial.print("AngleY= ");
  Serial.println(y);
  
  Serial.print("AngleZ= ");
  Serial.println(z);
  Serial.println("-----------------------------------------");

}

void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}

void distanceSens() {

  /*digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;*/

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 28.5;

  Serial.print("Distance: ");
  Serial.println(distance);
  
}

void distanceSens2() {

  /*digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = duration2 * 0.034 / 2;*/

  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = (duration2/2) / 28.5;

  Serial.print("Distance2: ");
  Serial.println(distance2);
  
}

void distanceSens3() {

  /*digitalWrite(trigPin3, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin3, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin3, LOW);
  duration3 = pulseIn(echoPin3, HIGH);
  distance3 = duration3 * 0.034 / 2;*/

  digitalWrite(trigPin3, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin3, LOW);
  duration3 = pulseIn(echoPin3, HIGH);
  distance3 = (duration3/2) / 28.5;


  Serial.print("Distance3: ");
  Serial.println(distance3);
  
}

void distanceSens4() {

  /*digitalWrite(trigPin4, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin4, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin4, LOW);
  duration4 = pulseIn(echoPin4, HIGH);
  distance4 = duration4 * 0.034 / 2;*/

  digitalWrite(trigPin4, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin4, LOW);
  duration4 = pulseIn(echoPin4, HIGH);
  distance4 = (duration4/2) / 28.5;

  Serial.print("Distance4: ");
  Serial.println(distance4);
  
}

void ultrasonicDetect()  {



  //FRONT ULTRASONIC SENSOR===========================
  if(distance3 > 0 || distance4 > 0){
    if(distance3 <= (70+sensorEqualizer)){
      if(hapticState == 0){
        analogWrite(53, 255);
      } 
      obstacleDetectMataasTrig = "ON";
    }

    else if(distance4 <= 70){
      if(hapticState == 0){
        analogWrite(53, 255);
      } 
      obstacleDetectMababaTrig = "ON";
    }

    else{
      analogWrite(53, 0);    
    }
  }


  //LEFT ULTRASONIC SENSOR==============================
  if(distance2 > 0 && distance2 <= 70){
    
    if(hapticState == 0){
      analogWrite(52, 255);
    } 
    obstacleDetectKaliwaTrig = "ON";    
  }

  else{
    analogWrite(52, 0);;    
  }    

  //RIGHT ULTRASONIC SENSOR==================================
  if(distance > 0 && distance <= 70){
    if(hapticState == 0){
      analogWrite(51, 255);
    } 
    obstacleDetectKananTrig = "ON";
  }

  else{
    analogWrite(51, 0);;    
  }

}



