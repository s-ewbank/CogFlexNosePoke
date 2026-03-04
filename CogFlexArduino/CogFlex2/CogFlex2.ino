/*
  For stage 3 of reversal learning task training (shaping 2)
  All lights are on. Center port poke initiates trial. For 5 s after trial initiation, left or right port pokes get a reward.
  To change rewarded side: 
  COMMENT or UNCOMMENT reward() call from "if (pokePort1_poke==1)" or "if (pokePort3_poke==1)" for loop in readPokes() !
  
  Written 22-04-18 by Sedona Ewbank
  Updated 22-07-22 
  Updated 22-07-29 both ports get reward
  Updated 23-01-15 to have initiation light and to match shaping 1 file except for responseTime=5
  Updated 23-03-05 to have audio cue for initiation
*/

int count = 0;
int rewardCount = 0;
int rightRewarded = 0;

//Only using 3 ports - from rat's view, Left is Port 1, Center is 2, and Right is 3...
//Analog pins for poke hole photoresistors
int pokePort1PR=A4; //left - prints first
int pokePort2PR=A3; // middle - prints second
int pokePort3PR=A5; // right - prints third


//LED lights for poke holes
int pokePort1LED=5;
int pokePort2LED=6;
int pokePort3LED=4;
int initLED=7;

// Tone settings
int tonePin = 12;
int toneFreq = 5000;

//Initializing variable for photoresistor readings
int pokePort1PR_val = 100;
int pokePort2PR_val = 100;
int pokePort3PR_val = 100;

//True or false - animal currently poking hole
int pokePort1_poke = 0;
int pokePort2_poke = 0;
int pokePort3_poke = 0;

//Initializing for poke scaling
long pokePort1_scale = 100;
long pokePort2_scale = 100;
long pokePort3_scale = 100;

//other variables
int threshold = 85; //poke photoresistor threshold
int nPokesTotal = 0;
int nRewards = 0;
int nErrors = 0;
int trialInitiated = 0;
int responseTime = 60; //seconds
int pokeSampleRate=100; //milliseconds between timepoints for sensing nose pokes
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long previousUpdateMillis = 0;
unsigned long lastInitiated = 0;
unsigned long lastChoice = 0;

//Solenoid pin and reward time
int solenoidPin = 8;
int rewardTime = 250;

void setup() {
  Serial.begin(9600);
  pinMode(pokePort1PR, INPUT);
  pinMode(pokePort2PR, INPUT);
  pinMode(pokePort3PR, INPUT);
  pinMode(pokePort1LED, OUTPUT);
  pinMode(pokePort2LED, OUTPUT);
  pinMode(pokePort3LED, OUTPUT);
  pinMode(tonePin, OUTPUT); // ADDED 3/5/23
  pinMode(initLED, OUTPUT); //ADDED 1/13/23
  pinMode(solenoidPin,OUTPUT);
  //digitalWrite(solenoidPin,LOW);
  digitalWrite(pokePort1LED, HIGH);
  digitalWrite(pokePort2LED, HIGH); 
  digitalWrite(pokePort3LED, HIGH); 
  digitalWrite(initLED,LOW); //ADDED 1/13/23

  
}

void loop() {
  currentMillis = millis();
  readPokes();
}


void readPokes() {
  pokePort1PR_val = abs(analogRead(pokePort1PR));
  pokePort2PR_val = abs(analogRead(pokePort2PR));
  pokePort3PR_val = abs(analogRead(pokePort3PR));
  if ((100*pokePort1PR_val/pokePort1_scale < 80) & (100*pokePort1PR_val/pokePort1_scale < (10+100*pokePort3PR_val/pokePort3_scale)) & (100*pokePort1PR_val/pokePort1_scale < (10+100*pokePort2PR_val/pokePort2_scale))){
    pokePort1_poke=1;
  } else {
    pokePort1_poke=0;
  }
  if ((100*pokePort2PR_val/pokePort2_scale < 80) & (100*pokePort2PR_val/pokePort2_scale < (10+100*pokePort1PR_val/pokePort1_scale)) & (100*pokePort2PR_val/pokePort2_scale < (10+100*pokePort3PR_val/pokePort3_scale))){
    pokePort2_poke=1;
  } else {
    pokePort2_poke=0;
  }
  if ((100*pokePort3PR_val/pokePort3_scale < 80) & (100*pokePort3PR_val/pokePort3_scale < (10+100*pokePort1PR_val/pokePort1_scale)) & (100*pokePort3PR_val/pokePort3_scale < (10+100*pokePort2PR_val/pokePort2_scale))){
    pokePort3_poke=1;
  } else {
    pokePort3_poke=0;
  }
  if (currentMillis - previousUpdateMillis >= pokeSampleRate) {
    Serial.print(currentMillis);
    Serial.print(",");
//    Serial.print(100*pokePort1PR_val/pokePort1_scale);
//    Serial.print(",");
//    Serial.print(100*pokePort2PR_val/pokePort2_scale);
//    Serial.print(",");
//    Serial.print(100*pokePort3PR_val/pokePort3_scale);
//    Serial.print(",");
    Serial.print(pokePort1_poke);
    Serial.print(",");
    Serial.print(pokePort2_poke);
    Serial.print(",");
    Serial.print(pokePort3_poke);
    Serial.print(",");
    Serial.print(nRewards);
    Serial.print(",");
    Serial.println(nErrors);
    previousUpdateMillis = millis();
    if (count<5){
      count=count+1;
    } else if (count<6){
      count=count+1;
      pokePort1_scale=pokePort1PR_val;
      pokePort2_scale=pokePort2PR_val;
      pokePort3_scale=pokePort3PR_val;
    } else if (count<30){
      count=count+1;
      pokePort1_scale=pokePort1PR_val+pokePort1_scale;
      pokePort2_scale=pokePort2PR_val+pokePort2_scale;
      pokePort3_scale=pokePort3PR_val+pokePort3_scale;
    } else if (count<31){
      count=50;
      pokePort1_scale=pokePort1_scale/25;
      pokePort2_scale=pokePort2_scale/25;
      pokePort3_scale=pokePort3_scale/25;
    }
    if (previousUpdateMillis*.001-lastInitiated*.001>responseTime){
      trialInitiated=0;
      digitalWrite(initLED,LOW); //ADDED 1/13/23
      digitalWrite(pokePort2LED, HIGH); //ADDED 1/13/23
      noTone(tonePin);
    }
    if (pokePort1_poke==1){
      reward();
    }
    if (pokePort2_poke==1){
      rewardCount=0;
      initiation();
    }
    if (pokePort3_poke==1){
      //reward();
    }
  }
}

void initiation() {
  currentMillis=millis();
  if (currentMillis<5000){
    //do nothing
  } else if ((trialInitiated==0) & (currentMillis - lastChoice > 500)){ //EDITED 1/13/23
    trialInitiated=1;
    digitalWrite(initLED,HIGH); //ADDED 1/13/23
    tone(tonePin, toneFreq);
    //digitalWrite(pokePort2LED, LOW); //ADDED 1/13/23
    lastInitiated=millis();
  }
}

void error() {
  currentMillis=millis();
  if (currentMillis<5000){
    //do nothing
  } else if ((trialInitiated==1) & (currentMillis - lastInitiated > 200)){
      noTone(tonePin);
      digitalWrite(initLED,LOW); //ADDED 1/13/23
      digitalWrite(pokePort2LED, HIGH); //ADDED 1/13/23
      nErrors = nErrors+1;
      trialInitiated=0;
      lastChoice=millis(); //ADDED 1/13/23
  }
}

void reward() {
  currentMillis=millis();
  if (currentMillis<5000){
    //do nothing
  } else if ((rewardCount==0) && (trialInitiated==1) && (currentMillis - lastInitiated > 200)){
      currentMillis=millis();
      noTone(tonePin);
      digitalWrite(initLED,LOW); //ADDED 1/13/23
      digitalWrite(pokePort2LED, HIGH); //ADDED 1/13/23
      digitalWrite(solenoidPin,HIGH);
      delay(rewardTime);
      digitalWrite(solenoidPin,LOW);
      nRewards = nRewards + 1;
      rewardCount=1;
      trialInitiated=0;
      lastChoice=millis(); //ADDED 1/13/23
      
  }
}
