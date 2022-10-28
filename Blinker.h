#ifndef Blinker_h
#define Blinker_h

class Blinker {
private:
  const unsigned long* _durations{ nullptr };
  unsigned int _count;
  int _index;
  unsigned long _startMs;
  unsigned long _durationMs;
  double _speedMultiplier;
  uint8_t _pin;
  uint8_t _state;
  bool _running = false;
public:
  Blinker() {}
  void Begin(uint8_t pin, uint8_t startVal) {
    _pin = pin;
    _state = startVal;
    pinMode(_pin, OUTPUT);
  };

  void Start(const unsigned long* durations, unsigned int count, unsigned int speedPercent = 100) {
    _durations = durations;
    _count = count;
    _speedMultiplier = 100.0 / speedPercent;
    _index = -1;
    _startMs = 0;
    _durationMs = 0;
    digitalWrite(_pin, _state);
    _running = true;
  }

  void Stop() {
    digitalWrite(_pin, LOW);
    _running = false;
  }
  void Resume() {
    if (_durations != nullptr)
      _running = true;
  }

  void Update() {
    if (!_running) return;

    if (auto frameTimeMs = millis(); frameTimeMs - _startMs >= _durationMs) {
      if (++_index == _count)
        _index = 0;

      _state = _state == HIGH ? LOW : HIGH;
      digitalWrite(_pin, _state);

      _durationMs = _durations[_index] * _speedMultiplier;
      _startMs = frameTimeMs;
    }
  }
};
#endif