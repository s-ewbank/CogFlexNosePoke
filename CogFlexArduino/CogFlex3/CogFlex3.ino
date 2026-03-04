/*
  For stage 4 of reversal learning task training (test)
  All port lights are on. Center port poke initiates trial. For 5 s after trial initiation, right port pokes earn a reward and left
  port pokes are recorded as an error with a 30 second penalty.
  Written 22-04-18 by Sedona Ewbank
  Updated 22-06-13 to be reversal test
  Updated 22-06-19 to print correct ports and have different variables for simple discrim 
  Updated 22-11-23 to be probabilistic and have side choosing
  vs reversal rewards and errors
  Updated 22-07-31 to fix timeout issue
  Updated 23-03-18 to have multiple reversals
  Updated 23-05-22 to change reversal criterion to 8/10 running window correct, and smaller reward
  Updated 23-06-13 to have output to arduino pins 9, 10, and 11 for BNC IO board of open-ephys system
*/

//CHANGE THIS to change which side is initially rewarded
int rightInitiallyRewarded = 0; //Change to 0 if the left should be rewarded initially!!!

int count = 0;
int rewardCount = 0;

//Only using 3 ports - from rat's view, Left is Port 1, Center is 2, and Right is 3...
//Analog pins for poke hole photoresistors
int pokePort1PR=A4; //left - prints first
int pokePort2PR=A3; // middle - prints second
int pokePort3PR=A5; // right - prints third

//Pins for TTL output to BNC IO board
int rewardBNCPin=10;
int punishBNCPin=9;
int reverseBNCPin=11;

//LED lights for poke holes
int pokePort1LED=5;
int pokePort2LED=6;
int pokePort3LED=4;
int initLED=7;

// Tone settings
int tonePin = 12;
int toneFreq = 5000;
int toneFreq2 = 9000;

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
int LEDcalibrated = 0;

//other variables
int reversals = 0;
int reversed = 0;
int nPokesTotal = 0;
int nCorrect_Rewarded_SD = 0; //correct pokes for simple discrimination that received reward
int nCorrect_Punished_SD = 0; //correct pokes for simple discrimination that received penalty
int nErrors_Rewarded_SD = 0; //errors for simple discrimination that received reward
int nErrors_Punished_SD = 0; //errors for simple discrimination that received penalty
int nCorrect_Rewarded_RL = 0; //correct pokes for reversal that recieved reward
int nCorrect_Punished_RL = 0; //correct pokes for reversal that recieved penalty
int nErrors_Rewarded_RL = 0; //errors for reversal that received reward
int nErrors_Punished_RL = 0; //errors for reversal that received penalty
int consecCorrect = 0;
int consecCorrect_RL = 0;
int outcome_10ago = 0;
int outcome_9ago = 0;
int outcome_8ago = 0;
int outcome_7ago = 0;
int outcome_6ago = 0;
int outcome_5ago = 0;
int outcome_4ago = 0;
int outcome_3ago = 0;
int outcome_2ago = 0;
int outcome_1ago = 0;
int outcome_sum = 0;
int trialInitiated = 0;
unsigned long responseTime = 60000;
int criterion = 8; //correct outcomes out of the last 10 trials required for reversal
int threshold = 75; //poke photoresistor threshold
int probThreshold = 90; //for probabilistic reversal learning - probability (out of 100) of rewarded side being rewarded
unsigned long prob; //random number that decides for each trial whether reward is given
unsigned long errorPenalty = 5000; //milliseconds to prevent additional trials if animal pokes wrong hole.
//unsigned long errorPenalty = 100; //milliseconds to prevent additional trials if animal pokes wrong hole.
int pokeSampleRate=100; //milliseconds between timepoints for sensing nose pokes
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long previousUpdateMillis = 0;
unsigned long lastChoiceMillis = 0;
unsigned long lastInitiated = 0;
unsigned long lastReversedMillis = 0;

//Solenoid pin and reward time
int solenoidPin = 13; //used to be 8 but something got stuck in there
//int rewardTime = 250;
int rewardTime = 100;

void setup() {
  Serial.begin(9600);
  pinMode(pokePort1PR, INPUT);
  pinMode(pokePort2PR, INPUT);
  pinMode(pokePort3PR, INPUT);
  pinMode(pokePort1LED, OUTPUT);
  pinMode(pokePort2LED, OUTPUT);
  pinMode(pokePort3LED, OUTPUT);
  pinMode(solenoidPin,OUTPUT);
  pinMode(initLED,OUTPUT);
  pinMode(punishBNCPin,OUTPUT);
  pinMode(rewardBNCPin,OUTPUT);
  pinMode(reverseBNCPin,OUTPUT);
  //digitalWrite(solenoidPin,LOW);
  digitalWrite(pokePort1LED, HIGH);
  digitalWrite(pokePort2LED, HIGH);
  digitalWrite(pokePort3LED, HIGH);
  digitalWrite(initLED,LOW);
  digitalWrite(punishBNCPin,LOW);
  digitalWrite(rewardBNCPin,LOW);
  digitalWrite(reverseBNCPin,LOW);
}

void loop() {

  currentMillis = millis();
  readPokes();
}

void readPokes() {
  if ((outcome_sum < criterion) && (reversed==0)) {
//    if ((lastReversedMillis - millis()) < 10) {
//        outcome_1ago = 0;
//        outcome_2ago = 0;
//        outcome_3ago = 0;
//        outcome_4ago = 0;
//        outcome_5ago = 0;
//        outcome_6ago = 0;
//        outcome_7ago = 0;
//        outcome_8ago = 0;
//        outcome_9ago = 0;
//        outcome_10ago = 0;
//      }
    pokePort1PR_val = abs(analogRead(pokePort1PR));
    pokePort2PR_val = abs(analogRead(pokePort2PR));
    pokePort3PR_val = abs(analogRead(pokePort3PR));
    outcome_sum = outcome_1ago + outcome_2ago + outcome_3ago + outcome_4ago + outcome_5ago + outcome_6ago + outcome_7ago + outcome_8ago + outcome_9ago + outcome_10ago;
    prob = random(0,100);
    if ((100*pokePort1PR_val/pokePort1_scale < 85) & (100*pokePort1PR_val/pokePort1_scale < 100*pokePort3PR_val/pokePort3_scale) & (100*pokePort1PR_val/pokePort1_scale+5 < 100*pokePort2PR_val/pokePort2_scale)){
      pokePort1_poke=1;
    } else {
      pokePort1_poke=0;
    }
    if ((100*pokePort2PR_val/pokePort2_scale < 85) & (100*pokePort2PR_val/pokePort2_scale < 100*pokePort1PR_val/pokePort1_scale) & (100*pokePort2PR_val/pokePort2_scale < 100*pokePort3PR_val/pokePort3_scale)){
      pokePort2_poke=1;
    } else {
      pokePort2_poke=0;
    }
    if ((100*pokePort3PR_val/pokePort3_scale < 85) & (100*pokePort3PR_val/pokePort3_scale < 100*pokePort1PR_val/pokePort1_scale) & (100*pokePort3PR_val/pokePort3_scale+5 < 100*pokePort2PR_val/pokePort2_scale)){
      pokePort3_poke=1;
    } else {
      pokePort3_poke=0;
    }
    if (currentMillis - previousUpdateMillis >= pokeSampleRate) {
     Serial.print(currentMillis);
     Serial.print(",");
    //   Serial.print(100*pokePort1PR_val/pokePort1_scale);
    //   Serial.print(",");
    //   Serial.print(100*pokePort2PR_val/pokePort2_scale);
    //   Serial.print(",");
    //   Serial.println(100*pokePort3PR_val/pokePort3_scale);
    //  Serial.print(",");
     Serial.print(pokePort1_poke);
     Serial.print(",");
     Serial.print(pokePort2_poke);
     Serial.print(",");
     Serial.print(pokePort3_poke);
     Serial.print(",");
     Serial.print(nCorrect_Rewarded_SD);
     Serial.print(",");
     Serial.print(nCorrect_Punished_SD);
     Serial.print(",");
     Serial.print(nErrors_Punished_SD);
     Serial.print(",");
     Serial.print(nErrors_Rewarded_SD);
     Serial.print(",");
     Serial.print(nCorrect_Rewarded_RL);
     Serial.print(",");
     Serial.print(nCorrect_Punished_RL);
     Serial.print(",");
     Serial.print(nErrors_Punished_RL);
     Serial.print(",");
     Serial.print(nErrors_Rewarded_RL);
     Serial.print(",");
     Serial.println(reversals);
//      Serial.print(",");
//      Serial.println(outcome_sum);
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
      if (previousUpdateMillis*.001-lastInitiated*.001>.001*responseTime){
      trialInitiated=0;
      digitalWrite(initLED,LOW);
      noTone(tonePin);
      }
      if (pokePort1_poke==1){
        if (rightInitiallyRewarded==1){
          incorrect();
        } else {
          correct();
        }
      }
      if (pokePort2_poke==1){
        rewardCount=0;
        initiation();
      }
      if (pokePort3_poke==1){
        if (rightInitiallyRewarded==1){
          correct();
        } else {
          incorrect();
        }
      }
    }
  } else {
    if (reversed==0) {
      reversals = reversals+1;
      outcome_1ago = 0;
      outcome_2ago = 0;
      outcome_3ago = 0;
      outcome_4ago = 0;
      outcome_5ago = 0;
      outcome_6ago = 0;
      outcome_7ago = 0;
      outcome_8ago = 0;
      outcome_9ago = 0;
      outcome_10ago = 0;
      outcome_sum = 0;
      lastReversedMillis = millis();
    }
    reversed = 1;
    digitalWrite(reverseBNCPin,HIGH);
    readPokes2();
  }
}

void readPokes2() {
  pokePort1PR_val = abs(analogRead(pokePort1PR));
  pokePort2PR_val = abs(analogRead(pokePort2PR));
  pokePort3PR_val = abs(analogRead(pokePort3PR));
  outcome_sum = outcome_1ago + outcome_2ago + outcome_3ago + outcome_4ago + outcome_5ago + outcome_6ago + outcome_7ago + outcome_8ago + outcome_9ago + outcome_10ago;
  prob = random(0,100);
//      if ((lastReversedMillis - millis()) < 800) {
//        outcome_1ago = 0;
//        outcome_2ago = 0;
//        outcome_3ago = 0;
//        outcome_4ago = 0;
//        outcome_5ago = 0;
//        outcome_6ago = 0;
//        outcome_7ago = 0;
//        outcome_8ago = 0;
//        outcome_9ago = 0;
//        outcome_10ago = 0;
//      }
  if ((100*pokePort1PR_val/pokePort1_scale < 85) & (100*pokePort1PR_val/pokePort1_scale < 100*pokePort3PR_val/pokePort3_scale) & (100*pokePort1PR_val/pokePort1_scale+5 < 100*pokePort2PR_val/pokePort2_scale)){
    pokePort1_poke=1;
  } else {
    pokePort1_poke=0;
  }
  if ((100*pokePort2PR_val/pokePort2_scale < 85) & (100*pokePort2PR_val/pokePort2_scale < 100*pokePort1PR_val/pokePort1_scale) & (100*pokePort2PR_val/pokePort2_scale < 100*pokePort3PR_val/pokePort3_scale)){
    pokePort2_poke=1;
  } else {
    pokePort2_poke=0;
  }
  if ((100*pokePort3PR_val/pokePort3_scale < 85) & (100*pokePort3PR_val/pokePort3_scale < 100*pokePort1PR_val/pokePort1_scale) & (100*pokePort3PR_val/pokePort3_scale+5 < 100*pokePort2PR_val/pokePort2_scale)){
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
    Serial.print(nCorrect_Rewarded_SD);
    Serial.print(",");
    Serial.print(nCorrect_Punished_SD);
    Serial.print(",");
    Serial.print(nErrors_Punished_SD);
    Serial.print(",");
    Serial.print(nErrors_Rewarded_SD);
    Serial.print(",");
    Serial.print(nCorrect_Rewarded_RL);
    Serial.print(",");
    Serial.print(nCorrect_Punished_RL);
    Serial.print(",");
    Serial.print(nErrors_Punished_RL);
    Serial.print(",");
    Serial.print(nErrors_Rewarded_RL);
    Serial.print(",");
    Serial.println(reversals);
//    Serial.print(",");
//    Serial.println(outcome_sum);
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
    
    if (previousUpdateMillis*.001-lastInitiated*.001>.001*responseTime){
      trialInitiated=0;
      digitalWrite(initLED,LOW);
      noTone(tonePin);
      }
      if (pokePort1_poke==1){
        if (rightInitiallyRewarded==1){
          correct();
        } else {
          incorrect();
        }
      }
      if (pokePort2_poke==1){
        rewardCount=0;
        initiation();
      }
      if (pokePort3_poke==1){
        if (rightInitiallyRewarded==1){
          incorrect();
        } else {
          correct();
        }
      }
    if (outcome_sum >= criterion) {
      reversals = reversals+1;
      consecCorrect_RL = 0;
      lastReversedMillis = millis();
      if (reversed == 1) {
        outcome_1ago = 0;
        outcome_2ago = 0;
        outcome_3ago = 0;
        outcome_4ago = 0;
        outcome_5ago = 0;
        outcome_6ago = 0;
        outcome_7ago = 0;
        outcome_8ago = 0;
        outcome_9ago = 0;
        outcome_10ago = 0;
        outcome_sum = 0;
      }
      reversed = 0;
      digitalWrite(reverseBNCPin,LOW);
      readPokes();
    }
  }
}

void initiation() {
  currentMillis=millis();
  if (currentMillis<5000){
    //do nothing
  } else if (trialInitiated==0) {
    //digitalWrite(initLED,HIGH);
    tone(tonePin, toneFreq); //UNCOMMENT THIS
    trialInitiated=1;
    lastInitiated=millis();
  }
}


void incorrect() {
  currentMillis=millis();
  if (currentMillis<5000  or lastChoiceMillis-currentMillis<2000){
    //do nothing
  } else if (trialInitiated==1) {
      if (reversed==0){
        if (prob<probThreshold) {
          nErrors_Punished_SD = nErrors_Punished_SD+1;
          penalty();
        } else {
          nErrors_Rewarded_SD = nErrors_Rewarded_SD+1;
          reward();
        }
      }
      else {
        if (prob<probThreshold) {
          nErrors_Punished_RL = nErrors_Punished_RL+1;
          penalty();
        } else {
          nErrors_Rewarded_RL = nErrors_Rewarded_RL+1;
          reward();
        }
      }
//      if ((consecCorrect < cCTC) && (reversed==0)) {
//        consecCorrect = 0;
//      }
//      if ((consecCorrect_RL < cCTC) && (reversed==1)) {
//        consecCorrect_RL = 0;
//      }
  }
}

void penalty() {
  currentMillis=millis();
  if (currentMillis<5000  or lastChoiceMillis-currentMillis<2000){
    //do nothing
  } else if (trialInitiated==1) {
      noTone(tonePin);
      tone(tonePin, toneFreq2);
      digitalWrite(initLED,LOW);
      digitalWrite(punishBNCPin,HIGH);
//      digitalWrite(pokePort1LED,LOW);
//      digitalWrite(pokePort2LED,LOW);
//      digitalWrite(pokePort3LED,LOW);
      delay(errorPenalty);
      digitalWrite(punishBNCPin,LOW);
      noTone(tonePin);
//      digitalWrite(pokePort1LED,HIGH);
//      digitalWrite(pokePort2LED,HIGH);
//      digitalWrite(pokePort3LED,HIGH);
      delay(200);
      lastChoiceMillis=millis();
      trialInitiated=0;
      outcome_10ago=outcome_9ago;
      outcome_9ago=outcome_8ago;
      outcome_8ago=outcome_7ago;
      outcome_7ago=outcome_6ago;
      outcome_6ago=outcome_5ago;
      outcome_5ago=outcome_4ago;
      outcome_4ago=outcome_3ago;
      outcome_3ago=outcome_2ago;
      outcome_2ago=outcome_1ago;
      outcome_1ago=0;
  }
}

void correct() {
  currentMillis=millis();
  if (currentMillis<5000 or lastChoiceMillis-currentMillis<2000){
    //do nothing
  } else if ((rewardCount==0) && (trialInitiated==1)){
      if (reversed==0){
        if (prob<probThreshold) {
          nCorrect_Rewarded_SD = nCorrect_Rewarded_SD + 1;
          reward();
        } else {
          nCorrect_Punished_SD = nCorrect_Punished_SD + 1;
          penalty();
        }
      } else {
        if (prob<probThreshold) {
          nCorrect_Rewarded_RL = nCorrect_Rewarded_RL + 1;
          reward();
        } else {
          nCorrect_Punished_RL = nCorrect_Punished_RL + 1;
          penalty();
        }
      }
//      if (reversed==0) {
//        consecCorrect = consecCorrect + 1;
//      } else if (reversed==1) {
//        consecCorrect_RL = consecCorrect_RL + 1;
//      }
  }
}

void reward() {
  currentMillis=millis();
  if (currentMillis<5000 or lastChoiceMillis-currentMillis<2000){
    //do nothing
  } else if ((rewardCount==0) && (trialInitiated==1)){
      noTone(tonePin);
      //digitalWrite(initLED,LOW);
      currentMillis=millis();
      lastChoiceMillis=millis();
      digitalWrite(solenoidPin,HIGH);
      digitalWrite(rewardBNCPin,HIGH);
      delay(rewardTime);
      digitalWrite(solenoidPin,LOW);
      digitalWrite(rewardBNCPin,LOW);
      rewardCount=1;
      trialInitiated=0;
      outcome_10ago=outcome_9ago;
      outcome_9ago=outcome_8ago;
      outcome_8ago=outcome_7ago;
      outcome_7ago=outcome_6ago;
      outcome_6ago=outcome_5ago;
      outcome_5ago=outcome_4ago;
      outcome_4ago=outcome_3ago;
      outcome_3ago=outcome_2ago;
      outcome_2ago=outcome_1ago;
      outcome_1ago=1;
  }
}
