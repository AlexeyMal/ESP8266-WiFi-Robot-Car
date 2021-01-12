// AMA 2020 combining different modes in one firmware
// Command "V" (beep) switches modes

/******************* WiFi Robot Remote Control Mode ********************/
#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h> 
#include <ArduinoOTA.h>

// connections for drive Motors
int PWM_A = D1;
int PWM_B = D2;
int DIR_A = D3;
int DIR_B = D4;

const int buzPin = D5;      // set digital pin D5 as buzzer pin (use active buzzer)
const int ledPin = D8; //D0; //D8;      // set digital pin D8 as LED pin (use super bright LED)
//const int wifiLedPin = D0;  // set digital pin D0 as indication, the LED turn on if NodeMCU connected to WiFi as STA mode
// ultrasonic setup:
const int trigPin = D6; // trig pin connected to Arduino's pin D6
const int echoPin = D7; // echo pin connected to Arduino's pin D7
//IR Sensor setup:
const int irRightPin = A0;  // signal right IR sensor 
const int irCenterPin = 3; //RX; // signal center IR sensor // !!! Disconnect IRSensor before flashing via Serial/USB
const int irLeftPin = D0; //D8; //D0;   // signal left IR sensor //D8 is bad: Boot fails if pulled HIGH
boolean stateRightIR = 0;  // state to store irCenterPin irRightPin
boolean stateCenterIR = 0; // state to store irCenterPin detected
boolean stateLeftIR = 0;   // state to store irCenterPin irLeftPin

String command;          // String to store app command state.
int distanceCm = 0;
int distanceHold = 20; //[cm] 15 avoidance mode
int distanceKeep = 15; // [cm] 10 follow mode
int SPEED = 1023;        // 330 - 1023.
int speed_Coeff = 2; //3;
int mode = 1; //AMA operation modi: 1 - remote control, 2 - obstacle avoidance, 3 - follow

ESP8266WebServer server(80);      // Create a webserver object that listens for HTTP request on port 80

unsigned long previousMillis = 0;

String sta_ssid = "";      // set Wifi networks you want to connect to
String sta_password = "";  // set password for Wifi networks


void setup(){
  Serial.begin(115200);    // set up Serial library at 115200 bps
  Serial.println();
  Serial.println("*WiFi Robot Remote Control Mode*");
  Serial.println("--------------------------------------");
 
  pinMode(buzPin, OUTPUT);      // sets the buzzer pin as an Output
  pinMode(ledPin, OUTPUT);      // sets the LED pin as an Output
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
//  pinMode(wifiLedPin, OUTPUT);  // sets the Wifi LED pin as an Output

  pinMode(irRightPin, INPUT);
  pinMode(irCenterPin, INPUT);
  pinMode(irLeftPin, INPUT);

  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  //digitalWrite(wifiLedPin, HIGH);
    
  // Set all the motor control pins to outputs
  pinMode(PWM_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(DIR_B, OUTPUT);
  
  // Turn off motors - Initial state
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);

  // set NodeMCU Wifi hostname based on chip mac address
  String chip_id = String(ESP.getChipId(), HEX);
  int i = chip_id.length()-4;
  chip_id = chip_id.substring(i);
  chip_id = "wificar-" + chip_id;
  String hostname(chip_id);
  
  Serial.println();
  Serial.println("Hostname: "+hostname);
/* //AMA do not connect as STA
  // first, set NodeMCU as STA mode to connect with a Wifi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.println("");
  Serial.print("Connecting to: ");
  Serial.println(sta_ssid);
  Serial.print("Password: ");
  Serial.println(sta_password);

  // try to connect with Wifi network about 10 seconds
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000) {
    delay(500);
    Serial.print(".");
    currentMillis = millis();
  }
*/
  // if failed to connect with Wifi network set NodeMCU as AP mode
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("*WiFi-STA-Mode*");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
//    digitalWrite(wifiLedPin, LOW);    // Wifi LED on when connected to Wifi as STA mode
    delay(3000);
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("");
    Serial.println("WiFi failed connected to " + sta_ssid);
    Serial.println("");
    Serial.println("*WiFi-AP-Mode*");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
//    digitalWrite(wifiLedPin, HIGH);   // Wifi LED off when status as AP mode
    delay(3000);
  }
 

  server.on ( "/", HTTP_handleRoot );       // call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound ( HTTP_handleRoot );    // when a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();                           // actually start the server
  
  ArduinoOTA.begin();                       // enable to receive update/uploade firmware via Wifi OTA
}


void loop() {
    ArduinoOTA.handle();          // listen for update OTA request from clients
    server.handleClient();        // listen for HTTP requests from clients
    
      command = server.arg("State");          // check HTPP request, if has arguments "State" then saved the value
      if (command == "F") Forward();          // check string then call a function or set a value
      else if (command == "B") Backward();
      else if (command == "R") TurnRight();
      else if (command == "L") TurnLeft();
      else if (command == "G") ForwardLeft();
      else if (command == "H") BackwardLeft();
      else if (command == "I") ForwardRight();
      else if (command == "J") BackwardRight();
      else if (command == "S") {Stop(); mode = 1;}
      else if (command == "V") BeepHorn();
      else if (command == "W") TurnLightOn();
      else if (command == "w") TurnLightOff();
      else if (command == "0") SPEED = 330;
      else if (command == "1") SPEED = 400;
      else if (command == "2") SPEED = 470;
      else if (command == "3") SPEED = 540;
      else if (command == "4") SPEED = 610;
      else if (command == "5") SPEED = 680;
      else if (command == "6") SPEED = 750;
      else if (command == "7") SPEED = 820;
      else if (command == "8") SPEED = 890;
      else if (command == "9") SPEED = 960;
      else if (command == "q") SPEED = 1023;
      
    ModeAvoidance();
    ModeFollow();
    ModeTracking();

    //delay(0); //recommended to do wifi stuff, to avoid being reset by software or hardware watchdogs
    delay(100); //300 // motors draw the voltage down when starting, so wait a litte. Otherwise the sensor readings are wrong.
}

// function prototypes for HTTP handlers
void HTTP_handleRoot(void){
  server.send ( 200, "text/html", "" );       // Send HTTP status 200 (Ok) and send some text to the browser/client
  
  if( server.hasArg("State") ){
     Serial.println(server.arg("State"));
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

// function to move forward
void Forward(){ 
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);
}

// function to move backward
void Backward(){
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);
}

// function to turn right
void TurnRight(){
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);
}

// function to turn left
void TurnLeft(){
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED);
}

// function to move forward left
void ForwardLeft(){
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED/speed_Coeff);
}

// function to move backward left
void BackwardLeft(){
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, SPEED);
  analogWrite(PWM_B, SPEED/speed_Coeff);
}

// function to move forward right
void ForwardRight(){
  digitalWrite(DIR_A, HIGH);
  digitalWrite(DIR_B, HIGH);
  analogWrite(PWM_A, SPEED/speed_Coeff);
  analogWrite(PWM_B, SPEED);
}

// function to move backward left
void BackwardRight(){ 
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, SPEED/speed_Coeff);
  analogWrite(PWM_B, SPEED);
}

// function to stop motors
void Stop(){  
  digitalWrite(DIR_A, LOW);
  digitalWrite(DIR_B, LOW);
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);
}

// function to beep a buzzer
void BeepHorn(){

  // The App sends this command twice, a double beep. I want only one, so some time checking here:
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > 600) //else ignore the command //1000 600
  {  
  //mode switch
  mode++;
  if((mode == 2) || (mode == 3) || (mode == 4)) 
    TurnLightOn(); //the light indicates autonomuos modi
  if(mode > 4) {
    mode = 1;  //switching modi in a loop
    Stop(); 
    TurnLightOff();
    }

  for (int i = 0; i < mode ; i++){ //signalize the mode with the buzzer
  digitalWrite(buzPin, HIGH);
  delay(50);
  digitalWrite(buzPin, LOW);
  delay(50);
  }
  //delay(500); //80
  previousMillis = currentMillis;
  }
  //else ignore the command
}

// function to turn on LED
void TurnLightOn(){
  digitalWrite(ledPin, HIGH);
}

// function to turn off LED
void TurnLightOff(){
  digitalWrite(ledPin, LOW);
}

// returns the distance measured by the HC-SR04 distance sensor
int getDistance() {
  int echoTime;                   //variable to store the time it takes for a ping to bounce off an object
  int calcualtedDistance;         //variable to store the distance calculated from the echo time
  
  //send out an ultrasonic pulse that's 10ms long
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);

  echoTime = pulseIn(echoPin, HIGH);      //use the pulsein command to see how long it takes for the
                                          //pulse to bounce back to the sensor

  calcualtedDistance = echoTime / 58.26;  //calculate the distance of the object that reflected the pulse (half the bounce time multiplied by the speed of sound)
  return calcualtedDistance;              //send back the distance that was calculated
}

// AMA
void ModeAvoidance(){
  if(mode == 2) {
  
  //Stop();
  
  distanceCm=getDistance();     // variable to store the distance measured by the sensor  
  Serial.println(distanceCm);   // print the distance that was measured
  
  //if the distance is less than value of distanceHold, The robot will turn right then move forward
  if(distanceCm <= distanceHold){
    // turn right
    //TurnRight();
	  // turn left
	  TurnLeft();
    delay(400);
    Stop();
  }
  else {
    // move forward
    Forward();
  }
  
  }
}

// AMA
void ModeFollow(){
  if(mode == 3) {
  
  distanceCm=getDistance();     // variable to store the distance measured by the sensor  
  Serial.println(distanceCm);   // print the distance that was measured

  // if the distance object is too close then move backward
  if(distanceCm < distanceKeep && distanceCm >= 0){
    // move backward
    Backward();
  } 
  // if the distance object is too far then move forward
  if(distanceCm > (distanceKeep+5) && distanceCm < (distanceKeep+20)){
    // move forward
    Forward();
  } 
  // if no object is detected continues turn right to searching
  if(distanceCm > (distanceKeep+20)){
    // turn right
    TurnRight();
  }
  // if the distance object is fitted then keep stop
  if(distanceCm >= distanceKeep && distanceCm <= (distanceKeep+5)){
    // stop motor
    Stop();
  }
    
  }
}

// AMA
void ModeTracking(){
  //print the detected condition by 3 IR sensor
  //Serial.println("IR_Left:"+String(digitalRead(irLeftPin))+"\tIR_Center:"+String(digitalRead(irCenterPin))+"\tIR_Right:"+String(analogRead(irRightPin))); 
  //delay(400);
  
  if(mode == 4) {
    
  // This logical for state 3 IR sensor, state is TRUE if IR sensor on black line, otherwise FALSE if IR sensor on white 
  stateRightIR = (analogRead(irRightPin) > 512); 
  stateCenterIR = digitalRead(irCenterPin);
  stateLeftIR = digitalRead(irLeftPin);

  if((stateLeftIR==false) && (stateCenterIR==true) && (stateRightIR==false)){
    // move forward
    Forward();
  }
  else if((stateLeftIR==false) && (stateCenterIR==false) && (stateRightIR==true)){
    // turn right
    TurnRight();
  }
  else if((stateLeftIR==false) && (stateCenterIR==true) && (stateRightIR==true)){
    // turn right
    TurnRight();
  }
  else if((stateLeftIR==true) && (stateCenterIR==false) && (stateRightIR==false)){
    // turn left
    TurnLeft();
  }
  else if((stateLeftIR==true) && (stateCenterIR==true) && (stateRightIR==false)){
    // turn left
    TurnLeft();
  }
  else if((stateLeftIR==true) && (stateCenterIR==true) && (stateRightIR==true)){
    // move forward
    //Forward();
    // stop motor
    Stop();
  } 
  else { //all flase
    // stop motor
    Stop();
  }
  }
}
