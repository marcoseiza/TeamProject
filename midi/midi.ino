#include <amidino.h>
#include <Servo.h>
#include "SyncServo.h"
#include "Timer.h"
#include <FastLED.h>

const int kBassDrumNote = 24;
const int kSnareDrumNote = 26;
const int kToCowbellNote = 27;
const int kToSnareNote = 25;
const int kCowbellNote = 28;
const int kHiHatNote = 29;
const int kShakerNote = 21;

const int kBassDrumStrikeTime = 80;
const int kBassDrumRecoilTime = 80;
const int kHiHatStrikeTime = 100;
const int kHiHatRecoilTime = 100;

const int kShakerPin = 5;
const int kRightStickPin = 6;
const int kRightBasePin = 7;
const int kLeftStickPin = 2;


Timer bass_drum_timer;
Timer hihat_timer;

const int kShakerStartAngle = 98;
const int kShakerEndAngleEnd = kShakerStartAngle - 30;

// python3 serialmidi.py --serial_name=/dev/cu.usbmodem101 --midi_in_name="Arduino Bus 1" --midi_out_name="Arduino Bus 1" --baud 19200

MidiParser midi;

SyncServo shaker;
SyncServo right_stick;
SyncServo right_base;
SyncServo left_stick;

const int kRightStickStartAngle = 95;
const int kRightStickHitDiff = -15;
const int kRightStickHitAngle = kRightStickStartAngle + kRightStickHitDiff;
const int kLeftStickStartAngle = 85;
const int kLeftStickHitDiff = 13;
const int kLeftStickHitAngle = kLeftStickStartAngle + kLeftStickHitDiff;
const int kRightBaseSnareAngle = 160;
const int kRightBaseCowbellAngle = 145;
const int kRightStickCowbellStartAngle = 120;
const int kRightStickCowbellHitDiff = -15;
const int kRightStickCowbellHitAngle = kRightStickCowbellStartAngle - kRightStickCowbellHitDiff;

#define NUM_LEDS 64
#define LEDS_DATA_PIN 4
Timer led_animation_timer;
uint8_t current_led_ii = 0;
uint8_t led_hue = 0;
bool show_led = false;

CRGB leds[NUM_LEDS];

void handleNoteOn(uint8_t ch, uint8_t note, uint8_t vel) {
  Serial.println(note);
  show_led = true;
  switch (note) {
    case kBassDrumNote:
      changeBassDrumState(false);
      break;
    case kSnareDrumNote:
      handleStickOn();
      break;
    case kHiHatNote:
      changeHiHatState(false);
      break;
    case kShakerNote:
      shaker.write(shaker.target_angle() == kShakerEndAngleEnd ? kShakerStartAngle : kShakerEndAngleEnd);
      break;
    case kToCowbellNote:
      right_base.write(kRightBaseCowbellAngle);
      right_stick.write(kRightStickCowbellStartAngle);
      break;
    case kToSnareNote:
      right_base.write(kRightBaseSnareAngle);
      right_stick.write(kRightStickStartAngle);
      break;
    case kCowbellNote:
      right_stick.write(kRightStickCowbellHitAngle);
      break;
  }
}

void handleNoteOff(uint8_t ch, uint8_t note, uint8_t vel) {
  switch (note) {
    case kBassDrumNote:
      changeBassDrumState(true);
      break;
    case kSnareDrumNote:
      handleStickOff();
      break;
    case kHiHatNote:
      changeHiHatState(false);
      break;
    case kCowbellNote:
      right_stick.write(kRightStickCowbellStartAngle);
      break;
  }
}

void handleStickOn() {
  // right_stick.write(kRightStickHitAngle);
  // left_stick.write(kLeftStickHitAngle);
  right_base.write(kRightBaseCowbellAngle);
  right_stick.write(kRightStickCowbellStartAngle);
}

void handleStickOff() {
  // right_stick.write(kRightStickStartAngle);
  // left_stick.write(kLeftStickStartAngle);
  right_base.write(kRightBaseSnareAngle);
  right_stick.write(kRightStickCowbellHitAngle);
}

void changeBassDrumState(bool play) {
  //start down
  digitalWrite(9, LOW);                 //Disengage the Brake for Channel A
  digitalWrite(12, play ? LOW : HIGH);  //Sets direction of CH A
  analogWrite(3, 255);                  //Moves CH A

  bass_drum_timer.begin(play ? kBassDrumStrikeTime : kBassDrumRecoilTime);
}

void changeHiHatState(bool play) {
  digitalWrite(8, LOW);                 //Disengage the Brake for Channel B
  digitalWrite(13, play ? LOW : HIGH);  //Sets direction of CH B
  analogWrite(11, 255);                 //Moves CH B

  bass_drum_timer.begin(play ? kHiHatRecoilTime : kHiHatStrikeTime);
}

int fadeLEDsii = 0;

void fadeAllLEDs() {
  for (fadeLEDsii = 0; fadeLEDsii < NUM_LEDS; fadeLEDsii++) {
    leds[fadeLEDsii].nscale8(250);
  }
}


void setup() {
  //establish motor direction toggle pins
  pinMode(12, OUTPUT);  //CH A -- HIGH = forwards and LOW = backwards???
  pinMode(13, OUTPUT);  //CH B -- HIGH = forwards and LOW = backwards???

  //establish motor brake pins
  pinMode(9, OUTPUT);  //brake (disable) CH A
  pinMode(8, OUTPUT);  //brake (disable) CH B

  digitalWrite(9, LOW);  //ENABLE CH A
  digitalWrite(8, LOW);  //ENABLE CH B

  shaker.begin(kShakerPin, kShakerStartAngle);
  right_stick.begin(kRightStickPin, kRightStickStartAngle);
  right_base.begin(kRightBasePin, kRightBaseSnareAngle);
  left_stick.begin(kLeftStickPin, kLeftStickStartAngle);

  Serial.begin(19200);

  midi.setChannel(MIDI_OMNI);
  midi.setNoteOnHandler(handleNoteOn);
  midi.setNoteOffHandler(handleNoteOff);

  // FastLED.addLeds<WS2812, LEDS_DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.setBrightness(84);
  // FastLED.clearData();
  // led_animation_timer.setOnByDefault(true);
}

void loop() {
  while (Serial.available()) {
    midi.parse(Serial.read());
  }

  shaker.update();
  right_stick.update();
  left_stick.update();
  right_base.update();

  if (bass_drum_timer.done()) {
    digitalWrite(9, HIGH);  //Engage the Brake for Channel A
  }

  if (hihat_timer.done()) {
    digitalWrite(8, HIGH);  //Engage the Brake for Channel B
  }

  // if (led_animation_timer.done() && show_led) {
  //   leds[current_led_ii] = CHSV(led_hue++, 255, 255);
  //   current_led_ii++;
  //   led_animation_timer.begin(10);
  //   if (current_led_ii > NUM_LEDS) {
  //     current_led_ii = 0;
  //     show_led = false;
  //   }
  // }
  // FastLED.show();
  // fadeAllLEDs();
}
