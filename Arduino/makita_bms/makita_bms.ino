#include <Makita.h>

 Makita<5> owi=Makita<5>();
void setup()
{
  Serial.begin(9600);
  pinMode(5,INPUT);
  pinMode(13, OUTPUT);

  owi.init();
  owi.set_overdischarge(25);
  owi.set_overload(25);
  owi.set_cycle_count(50);
  owi.set_health(25);
  owi.set_extended(true);
  owi.set_error(6);
}

void loop()
{
  owi.reset_voltages();
 while(true){
  //process makita rom commands, no time to wait for other stuff
  if (owi.rom_command()) {
  }
 }
  
  
}
