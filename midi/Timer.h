class Timer {
  int _time = 0;
  int _cur_time = 0;

  bool _on_by_default = false;
public:
  Timer(bool on_by_default = false) {
    _on_by_default = on_by_default;
  }
  ~Timer() {}

  void begin(int time) {
    _time = time;
    _cur_time = millis();
  }

  bool done() {
    if (_cur_time == 0 && _time == 0 && _on_by_default) return true;

    if (_cur_time > 0 && millis() - _cur_time >= _time) {
      _time = 0;
      _cur_time = 0;
      return true;
    }

    return false;
  }

  void setOnByDefault(bool on_by_default) {
    _on_by_default = on_by_default;
  }
};