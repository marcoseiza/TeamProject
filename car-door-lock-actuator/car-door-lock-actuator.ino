#include <analogWrite.h>

int cABreakPin = 13;
int cADirectionPin = 12;
int cAPowerPin = A1;

int kStrikeLength = 80;

void setup() {
  pinMode(cADirectionPin, OUTPUT);  // Channel A -- HIGH = forwards and LOW = backwards.
  pinMode(cABreakPin, OUTPUT);      // Channel A -- HIGH = engage and LOW = disengage.
}



void loop() {
  digitalWrite(cABreakPin, LOW);      // Disengage the Brake for Channel A.
  digitalWrite(cADirectionPin, LOW);  // Sets direction of Channel A.
  analogWrite(cAPowerPin, 255);       // Give power to Channel A.

  // Beat strike length.
  delay(kStrikeLength);

  // Stop actuator.
  digitalWrite(cABreakPin, HIGH);  // Engage the Brake for Channel A.
  analogWrite(cAPowerPin, 0);      // Stop giving power to Channel A.

  // Recoil time.
  delay(10);

  // Move actuator back.
  digitalWrite(cABreakPin, LOW);       // Disengage the Brake for Channel A.
  digitalWrite(cADirectionPin, HIGH);  // Sets direction of Channel A.
  analogWrite(cAPowerPin, 255);        // Give Power to Channel A.

  // Recoil time.
  delay(40);

  // Stop actuator.
  digitalWrite(cABreakPin, HIGH);  // Engage the Brake for Channel A.
  analogWrite(cAPowerPin, 0);      // Stop giving power to Channel A.

  delay(1000);
}
