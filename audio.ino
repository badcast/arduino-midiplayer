#include "melody.h"

MidiPlayer player(5); // select BUZZER (Dynamic) as 5 PIN for out

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  for (; !Serial;);
  Serial.println("Start player");
  player.setMelody(&WaveRemCry); // load melody - (Light Mister - Rem Cry https://soundcloud.com/light-mister/epic-dubstep-rock-rem-cry-remix)
  player.play();
}

void loop() {
  player.update();
}
