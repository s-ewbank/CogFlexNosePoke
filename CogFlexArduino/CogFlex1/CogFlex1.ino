/*
  For day 1 of reversal learning task training
  Rewards all nose pokes :)
  Dropped stepper motor for solenoid with liquid reward
  Written 22-04-18 by Sedona Ewbank
  Updated 22-06-14 with right arrangement of sides of ports
  Updated 23-04-17 to have tone and refractory period after reward
*/

int count = 0;
int rewardCount = 0;

//Only using 3 ports - from rat's view, Left is Port 1, Center is 2, and Right is 3...
//Analog pins for poke hole photoresistors
int pokePort1PR=A4; //left - prints first
int pokePort2PR=A3; // middle - prints second
int pokePort3PR=A5; // right - prints third

//LED lights for poke holes
int pokePort1LED=5;
int pokePort2LED=6;
int pokePort3LED=4;

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

//other variables
int threshold = 85; //poke photoresistor threshold
int nPokesTotal = 0;
int nRewards = 0;
int pokeSampleRate=100; //milliseconds between timepoints for sensing nose pokes
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long previousUpdateMillis = 0;
unsigned long lastRewardMillis = 0;

//Solenoid pin and reward time
int solenoidPin = 13;
int rewardTime = 400;
int refractory = 800;

void setup() {
  Serial.begin(9600);
  pinMode(pokePort1PR, INPUT);
  pinMode(pokePort2PR, INPUT);
  pinMode(pokePort3PR, INPUT);
  pinMode(pokePort1LED, OUTPUT);
  pinMode(pokePort2LED, OUTPUT);
  pinMode(pokePort3LED, OUTPUT);
  pinMode(solenoidPin,OUTPUT);
  //digitalWrite(solenoidPin,LOW);
  digitalWrite(pokePort1LED, HIGH);
  digitalWrite(pokePort2LED, HIGH);
  digitalWrite(pokePort3LED, HIGH);
  
}

void loop() {
  currentMillis = millis();
  readPokes();
}


void readPokes() {
  pokePort1PR_val = abs(analogRead(pokePort1PR));
  pokePort2PR_val = abs(analogRead(pokePort2PR));
  pokePort3PR_val = abs(analogRead(pokePort3PR));
  if (100*pokePort1PR_val/pokePort1_scale < 75){
    pokePort1_poke=1;
  } else {
    pokePort1_poke=0;
  }
  if (100*pokePort2PR_val/pokePort2_scale < 75){
    pokePort2_poke=1;
  } else {
    pokePort2_poke=0;
  }
  if (100*pokePort3PR_val/pokePort3_scale < 75){
    pokePort3_poke=1;
  } else {
    pokePort3_poke=0;
  }
  if (currentMillis - previousUpdateMillis >= pokeSampleRate) {
    Serial.print(currentMillis);
    Serial.print(",");
    //Serial.print(100*pokePort1PR_val/pokePort1_scale);
    //Serial.print(",");
    //Serial.print(100*pokePort2PR_val/pokePort2_scale);
    //Serial.print(",");
    //Serial.print(100*pokePort3PR_val/pokePort3_scale);
    //Serial.print(",");
    Serial.print(pokePort1_poke);
    Serial.print(",");
    Serial.print(pokePort2_poke);
    Serial.print(",");
    Serial.print(pokePort3_poke);
    Serial.print(",");
    Serial.println(nRewards);
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
    if (pokePort1_poke==1){
      rewardCount=0;
      reward();
    }
    if (pokePort2_poke==1){
      rewardCount=0;
      reward();
    }
    if (pokePort3_poke==1){
      rewardCount=0;
      reward();
    }
  }
}

void reward() {
  currentMillis=millis();
  if (currentMillis<5000){
    //do nothing
  } else if ((rewardCount==0) & (currentMillis-lastRewardMillis > 200)){
      currentMillis=millis();
      tone(tonePin,toneFreq);
      delay(50);
      noTone(tonePin);
      digitalWrite(solenoidPin,HIGH);
      //tone(tonePin,toneFreq);
      delay(rewardTime);
      //noTone(tonePin);
      digitalWrite(solenoidPin,LOW);
      nRewards = nRewards + 1;
      rewardCount=1;
      delay(refractory);
      lastRewardMillis=millis();
  }
}
