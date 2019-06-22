 
#include <SigFox.h>
#include <ArduinoLowPower.h>

// defines
#define DEBUG 1
#define us_num 4

#define DIST_US_BOTTOM 60
#define FULL_TANK_DIST DIST_US_BOTTOM / 2
#define DIST_FULL_SCALE (DIST_US_BOTTOM - FULL_TANK_DIST)

#define MIN_WEIGHT_FACTOR 1
#define MAX_WEIGHT_FACTOR 60
#define WEIGHT_FACTOR_FULL_SCALE (MAX_WEIGHT_FACTOR - MIN_WEIGHT_FACTOR)

#define MIN_LEVEL 0
#define MAX_LEVEL 100
#define LEVEL_FULL_SCALE (MAX_LEVEL - MIN_LEVEL)


// defines variables
long duration;
int distance[us_num];
int weight_factor; // a coefficient to give more weight to us measures with low distance
int weight_factor_sum;
int level;

int trigPin[us_num] = {6, 7, 8, 9};
int echoPin[us_num] = {10, 11, 12, 13};


volatile int alarm_source = 0;

void alarmEvent0() {
  alarm_source = 0;
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}

void setup() {

  delay(10000); //reprog timer :)

  for (int i =0; i < us_num; i++) { 
    pinMode(trigPin[i], OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin[i], INPUT);  // Sets the echoPin as an Input
  }

     Serial1.begin(115200);
    while (!Serial1) {}
    
    if (!SigFox.begin()) {
      //something is really wrong, try rebooting
      reboot();
    }

  // Send module to standby until we need to send a message
  //SigFox.end();
  if (DEBUG){
    SigFox.debug();
    Serial.println("ID  = " + SigFox.ID());
    Serial.println("ID  = " + SigFox.PAC());
  }
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE); 
}

void loop() {
  LowPower.sleep(30000);//3 mn sleep

  level = 0;
  weight_factor_sum = 0;
  
  for (int i =0; i < us_num; i++) {
    delayMicroseconds(30000);  // 0.0mM3 sec between each measure to make sure no rebounds of previous sensor occurs
    
    // Clears the trigPins
    digitalWrite(trigPin[i], LOW);
    Serial.print("US "); Serial.print(i); Serial.print("  -->  ");
    Serial.print("Pins "); Serial.print(trigPin[i]); Serial.print(" - ");Serial.print(echoPin[i]); Serial.print("  ///  ");
    delayMicroseconds(2);
    
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin[i], LOW);
    
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin[i], HIGH);
    
    // Calculating the distance
    distance[i] = duration*0.034/2;

    // Computing weight factor
    // weight=1 if sensor says 40cm (60cm = empty tank) / weight=10 if sensor says 20cm(0.33*60cm = full tank)
    weight_factor = int((float(WEIGHT_FACTOR_FULL_SCALE) / float(DIST_FULL_SCALE)) * (DIST_US_BOTTOM - distance[i])) + MIN_WEIGHT_FACTOR;
    if (weight_factor > MAX_WEIGHT_FACTOR)
        weight_factor = MAX_WEIGHT_FACTOR;
    if (weight_factor < MIN_WEIGHT_FACTOR)
        weight_factor = MIN_WEIGHT_FACTOR;
        
    weight_factor_sum += weight_factor;

    //Computing level
    level += (((int)(float(LEVEL_FULL_SCALE) / float(DIST_FULL_SCALE))) * (DIST_US_BOTTOM - distance[i])) * weight_factor;
    
    // Prints the distance on the Serial Monitor
    if (DEBUG){
      Serial.print("US #");
      Serial.print(i);
      Serial.print("- Distance: ");
      Serial.print(distance[i]);
      Serial.print("- Weight factor: ");
      Serial.print(weight_factor);
      Serial.print("- tmp Level: ");
      Serial.println(level);
      delay(1);
    }
  }
  if (DEBUG){
    Serial.println(weight_factor_sum);
  }

  level /= weight_factor_sum;
  if (level > MAX_LEVEL)
    level = MAX_LEVEL;
  if (level < MIN_LEVEL)
    level = MIN_LEVEL;
      
  if (DEBUG){
    Serial.print("LEVEL : ");
    Serial.println(level);
  }

    delay(100);

  // Compose a message as usual; the single bit transmission will be performed transparently
  // if the data we want to send is suitable
  SigFox.beginPacket();
  
  SigFox.write(level); 
  
  int ret = SigFox.endPacket();

  if (DEBUG){
    Serial.print("Status : ");
    Serial.println(ret);
  }
}
