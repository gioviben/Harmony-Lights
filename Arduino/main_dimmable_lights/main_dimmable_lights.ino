#include <AudioAnalyzer.h>

#define RED_LIGHT_PIN 12
#define YELLOW_LIGHT_PIN 8
#define GREEN_LIGHT_PIN 13

#define RED_LIGHT 0
#define YELLOW_LIGHT 1
#define GREEN_LIGHT 2

#define ON 0x0
#define OFF 0x1

#define BAND 1
#define THRESHOLD 680

Analyzer Audio = Analyzer(4, 5, 5);

int FreqVal[7];
int BPM = 130;

long unsigned int wait = (60000/BPM)-(20*7);
int freqzPrec = 0;
int freqzPrec2 = 0;
bool synchronizing = true;
int count = 0;

void setup() {
  Serial.begin(57600);
  Audio.Init();  //Init module

  pinMode(RED_LIGHT_PIN, OUTPUT);
  digitalWrite(RED_LIGHT_PIN, OFF);

  pinMode(YELLOW_LIGHT_PIN, OUTPUT);
  digitalWrite(YELLOW_LIGHT_PIN, OFF);

  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  digitalWrite(GREEN_LIGHT_PIN, OFF);
  
}

String bpmString = "";

void refreshBPM(int *BPM, long unsigned int *wait){
  while(Serial.available()){
    char ch = Serial.read();
    if(ch == '\0'){
      if(bpmString.length() > 1){
        Serial.println("Current BPM: " + bpmString);
        int bpmInt = bpmString.toInt();
        if(((*BPM) != bpmInt) && ((bpmInt >=50) && (bpmInt <=210))){
          *BPM = bpmInt;
          *wait = round((60000.0/(*BPM))-(20*7));
          Serial.println(*BPM);
          Serial.println(*wait);
        }
      }
      bpmString = "";
      /*
        synchronizing = true;
        freqzPrec = 0;
        freqzPrec2 = 0;
      */
    }else{
      bpmString += ch;
    }
  }
}

void loop() {

  Audio.ReadFreq(FreqVal);  //Return 7 values of 7 bands pass filiter
                            //Frequency(Hz):63  160  400  1K  2.5K  6.25K  16K
                            //FreqVal[]:      0    1    2    3    4    5    6

  
  refreshBPM(&BPM, &wait);
  
  while(synchronizing && ((max((freqzPrec - 100), 0)<THRESHOLD) || (max((FreqVal[BAND] - 100), 0)<THRESHOLD)) || (max((freqzPrec2 - 100), 0)<THRESHOLD)){
    //for (int i = 0; i < 7; i++) {
    //Serial.print(max((FreqVal[i]-100),0));
    //if(i<6)  Serial.print(",");
    //else Serial.println();
    //}  
    //Serial.println("Synchronizing");
    freqzPrec2 = freqzPrec;
    freqzPrec = FreqVal[BAND];
    Audio.ReadFreq(FreqVal);
    refreshBPM(&BPM, &wait);
  }

  synchronizing = false;

  if ((max((FreqVal[BAND] - 100), 0) >= THRESHOLD)) { //4          //700 --> -7.2 dB
    int randomLights = random(3);
    switch (randomLights) {
      case RED_LIGHT:
          digitalWrite(RED_LIGHT_PIN, ON);
          delay(140);
          digitalWrite(RED_LIGHT_PIN, OFF);
        break;
      case YELLOW_LIGHT:
          digitalWrite(YELLOW_LIGHT_PIN, ON);
          delay(140);
          digitalWrite(YELLOW_LIGHT_PIN, OFF);
        break;
      case GREEN_LIGHT:
          digitalWrite(GREEN_LIGHT_PIN, ON);
          delay(140);
          digitalWrite(GREEN_LIGHT_PIN, OFF);
      break;
    }   
  } else {
    count++;
    if (count > 1){
      count = 0;
      synchronizing = true;
      freqzPrec = 0;
      freqzPrec2 = 0;
    }
  } 
  //Serial.print(max((FreqVal[i]-100),0));
  //if(i<6)  Serial.print(",");
  //else Serial.println();
  delay(wait);
}
