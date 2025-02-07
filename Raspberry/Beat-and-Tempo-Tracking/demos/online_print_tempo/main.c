//OSX: gcc *.c ../../src/*.c -framework AudioToolbox
//Linux: gcc *.c ../../src/*.c -lasound -lm -lpthread -lrt

#include "Microphone.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
/*--------------------------------------------------------------------*/

int polishStringBPM(char *stringBPM){
  for(int i =0; i < strlen(stringBPM); ++i){
    if(stringBPM[i] == '.'){
      stringBPM[i] = '\0';
      return i;
    }
  }
}

int main(void)
{

  // Connect to the serial port
  int serial_port = open("/dev/ttyUSB0", O_RDWR);
  if(serial_port < 0){
      perror("Errore nell apertura della porta seriale");
      return 1;
  }

  struct termios tty;
  tcgetattr(serial_port, &tty);

  cfsetispeed(&tty, B57600);
  cfsetospeed(&tty, B57600);
  tty.c_cflag |= (CLOCAL | CREAD);
  tcsetattr(serial_port, TCSANOW, &tty);

  // Prepare to detect BPM

  Microphone* mic = mic_new();
  if(mic == NULL) {perror("Unable to create microphone object"); exit(-1);}
  
  BTT* btt = mic_get_btt(mic);
  btt_set_count_in_n(btt, 0);
  btt_set_tracking_mode(btt, BTT_ONSET_AND_TEMPO_TRACKING);
  
  auPlay((Audio*)mic);
  
  // Start BPM detecting

  double previusBPM = 0;
  double currentBPM = 0;
  double delta;
  char stringBPM[100];

  sleep(20);
  for(;;)
    {
      sleep(0.5);

      double tempo = btt_get_tempo_bpm(btt);
      currentBPM = round(tempo);

      delta = fabs(currentBPM - previusBPM);

      //fprintf(stderr, "Rounded: %f\r\n", currentBPM);

      if(delta >= 1.0){
          fprintf(stderr, "Delta >= 1: delta=%f\r\n", delta);
          fprintf(stderr, "Rounded: %f\r\n", currentBPM);
          sprintf(stringBPM, "%f", currentBPM);
          int len = polishStringBPM(stringBPM);
          char *polishedStringBPM = (char *)calloc(len+1, sizeof(char));
          strcpy(polishedStringBPM, stringBPM);
          fprintf(stderr, "Rounded polished: %s\r\n", stringBPM);
          write(serial_port, polishedStringBPM, len+1);
          free(polishedStringBPM);
      }

      previusBPM = currentBPM;

      //fprintf(stderr, "Original: %02f BPM\r\n", tempo);
    }
}
