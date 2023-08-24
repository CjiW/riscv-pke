/*
 * Below is the given application for lab5_2.
 * The goal of this app is to control the car via Bluetooth.
 */

#include "user_lib.h"
#include "util/types.h"
void delay(unsigned int time){
  unsigned int a = 0xfffff ,b = time;
  volatile unsigned int i,j;
  for(i = 0; i < a; ++i){
    for(j = 0; j < b; ++j){
      ;
    }
  }
}
int main(void) {
  printu("Hello world!\n");
  int i;
  int pid = fork();
  if(pid == 0)
  {
    while (1)
    {
      delay(3);
      printu("waiting for you!\n");
    }
  }
  else
  {
    while(1)
    {
      char temp = (char)uartgetchar();
      if(temp == 'q')
        break;
      car_control(temp);
    }
  }

  exit(0);

  return 0;
}
