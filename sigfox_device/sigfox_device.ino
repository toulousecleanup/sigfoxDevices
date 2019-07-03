 
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

#define RETRY_PER_US 6 // For each US, measure RETRY_PER_US times and average
#define DROP_COEF 1.3 // Measures for a US that are > (mean of measures for the US * DROP_COEF) are dropped


// defines variables
long duration;
int distance[us_num];
int weight_factor; // a coefficient to give more weight to us measures with low distance

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

  delay(25000); // This 25s delay allows you to reprog the Arduino before it goes in LowPower.sleep() :)

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

  int level = 0;
  int weight_factor_sum = 0;
  for (int a = 0; a < us_num; ++a)
    distance[a] = 0;
  
  for (int i =0; i < us_num; i++) {
    Serial.println(" "); Serial.print("US "); Serial.print(i); Serial.print("  -->  ");
    Serial.print("Pins "); Serial.print(trigPin[i]); Serial.print(" - ");Serial.print(echoPin[i]); Serial.println("  ///  ");

    int tmp_distance[RETRY_PER_US];
    for (int b = 0; b < RETRY_PER_US; ++b)
      distance[b] = 0;    
    
    for (int j =0; j < RETRY_PER_US; j++) {
      delayMicroseconds(50000);  // 50milisec between each measure to make sure no rebounds of previous sensor occurs
      
      // Clears the trigPins
      digitalWrite(trigPin[i], LOW);
      delayMicroseconds(2);
      
      // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(trigPin[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin[i], LOW);
      
      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(echoPin[i], HIGH);
      
      // Calculating the distance
      tmp_distance[j] = duration*0.034/2;
      if (DEBUG){
        Serial.print("    US #");
        Serial.print(i);
        Serial.print("- tmp Distance for measure #");
        Serial.print(j);        
        Serial.print(": ");
        Serial.println(tmp_distance[j]);
        delay(1);
      }      
    }

    // Filter tmp distance for the US and drop too high
    // Compute mean and drop measures above mean * DROP_COEF

    // Mean
    int us_mean=0;
    for (int id=0; id<RETRY_PER_US; id++){
      us_mean += tmp_distance[id]; 
    }
    us_mean /= RETRY_PER_US;
    Serial.print("us_mean: ");Serial.println(us_mean);
    

    // Drop too big measures for this US and compute average of the remaining measures
    int divider = 0;
    for (int idx=0; idx<RETRY_PER_US; idx++){
      if (tmp_distance[idx] <= us_mean * DROP_COEF) {
        distance[i] += tmp_distance[idx];
        divider += 1;
      }
      else {
        if (DEBUG){
          Serial.print("Droping measure #"); Serial.print(idx); Serial.print(" : "); Serial.print(tmp_distance[idx]); 
          Serial.print(" for it is > mean ("); Serial.print(us_mean); Serial.print(") * DROP_COEF ("); Serial.print(DROP_COEF); Serial.println(")");
        }
      }
    }   
    distance[i] /= divider; // Mean of remaining measures
    
    // Computing weight factor
    // weight=1 if sensor says 40cm (60cm = empty tank) / weight=10 if sensor says 20cm(0.33*60cm = full tank)
    weight_factor = int((float(WEIGHT_FACTOR_FULL_SCALE) / float(DIST_FULL_SCALE)) * (DIST_US_BOTTOM - distance[i])) + MIN_WEIGHT_FACTOR;
    if (weight_factor > MAX_WEIGHT_FACTOR)
        weight_factor = MAX_WEIGHT_FACTOR;
    if (weight_factor < MIN_WEIGHT_FACTOR)
        weight_factor = MIN_WEIGHT_FACTOR;
        
    weight_factor_sum += weight_factor;

    //Computing level:  weighed measured level
    float tmp_level = (DIST_US_BOTTOM - distance[i]);   
    if (int(tmp_level) > DIST_US_BOTTOM)
      tmp_level = DIST_US_BOTTOM;
    if (int(tmp_level) < 0)
      tmp_level = 0;
    tmp_level /= DIST_FULL_SCALE;
    tmp_level *= LEVEL_FULL_SCALE;
    tmp_level *= weight_factor;

    Serial.println(level);
    level += int(tmp_level);
    
    // Prints the distance on the Serial Monitor
    if (DEBUG){
      Serial.print("US #");
      Serial.print(i);
      Serial.print("- Averaged Distance (after eventual measures drop): ");
      Serial.print(distance[i]);
      Serial.print("- Weight factor: ");
      Serial.print(weight_factor);
      Serial.print("- tmp Level: ");
      Serial.println(tmp_level);
      Serial.print("- summed Level: ");
      Serial.println(level);
      delay(1);
    }
  }
  if (DEBUG){
    Serial.print("weight_factor_sum: ");
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
  
  SigFox.write(uint8_t(level)); 
  
  int ret = SigFox.endPacket();

  if (DEBUG){
    Serial.print("Status : ");
    Serial.println(ret);
    Serial.println("----------------------------------------\n");
  }

  LowPower.sleep(1800000); // in milliseconds <-> 30 mn sleep to save the battery before next measurement
}
