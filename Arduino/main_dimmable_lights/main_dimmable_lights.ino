#include <AudioAnalyzer.h>
#include <RBDdimmer.h>

#define USE_SERIAL Serial
#define outputPin 12

#define BAND 1
#define THRESHOLD 680


Analyzer Audio = Analyzer(4, 5, 5);

int FreqVal[7];
int BPM = 130;

dimmerLamp dimmer(outputPin);

long unsigned int wait = (60000/BPM)-(20*7);
int freqzPrec = 0;
int freqzPrec2 = 0;
bool synchronizing = true;
int count = 0;


void setup() {
  Serial.begin(57600);
  Audio.Init();  //Init module
  dimmer.begin(NORMAL_MODE, OFF);
  dimmer.setPower(50);

  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
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

  for (int i = 0; i < 7; i++) {
    if (max((FreqVal[BAND] - 100), 0) >= THRESHOLD) { //4          //700 --> -7.2 dB
      dimmer.setState(ON);
    } else if(i == BAND){
      count++;
      if (count > 1){
        count = 0;
        synchronizing = true;
        freqzPrec = 0;
        freqzPrec2 = 0;
        break;
      }
    } 
    //Serial.print(max((FreqVal[i]-100),0));
    //if(i<6)  Serial.print(",");
    //else Serial.println();
    delay(20);
    dimmer.setState(OFF);
  }
  delay(wait);
}
