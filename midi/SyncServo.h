#include <Servo.h>

class SyncServo {
private:
  Servo _servo;

  int _current_angle = 90;

  int _target_angle = 90;

  unsigned long _last_tick = 0;

  int _delay = 18;

  int _diff = 0;

  unsigned long _current_tick = 0;

public:
  SyncServo(){};
  ~SyncServo(){};

  void begin(int pin, int starting_angle) {
    _current_angle = starting_angle;
    _target_angle = starting_angle;
    _servo.attach(pin);
    _servo.write(starting_angle);
  }

  bool update() {
    if (_current_angle == _target_angle)
      return true;

    _current_tick = millis();
    if (_current_tick - _last_tick <= _delay) return false;
    _last_tick = _current_tick;

    _diff = 0;

    if (_target_angle > _current_angle) {
      _diff = 3;
    } else if (_target_angle < _current_angle) {
      _diff = -3;
    } else {
      return true;
    }

    _current_angle = _current_angle + _diff;

    _servo.write(_current_angle);

    if (abs(_current_angle - _target_angle) < _diff)
      return true;

    return false;
  }

  void write(int angle) {
    _target_angle = angle;
  }

  int target_angle() {
    return _target_angle;
  }
};