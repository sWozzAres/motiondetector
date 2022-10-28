#ifndef RgbLed_h
#define RgbLed_h

enum class RgbColour { Red,
                       Green,
                       Blue };
class RgbLed {
private:
  uint8_t _redPin;
  uint8_t _bluePin;
  uint8_t _greenPin;
  int _redVal;
  int _greenVal;
  int _blueVal;

  uint8_t _flashPin;
  unsigned long _flashStartMs;
  bool _flashing{ false };

public:
  RgbLed() {}
  void Begin(uint8_t redPin, uint8_t greenPin, uint8_t bluePin) {
    _redPin = redPin;
    _greenPin = greenPin;
    _bluePin = bluePin;
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);
    ChangeColour(255, 255, 255);
  }

  void ChangeFlashColour(RgbColour colour) {
    analogWrite(_flashPin, 255);
    _flashPin = PinFromColour(colour);
  }

  void StartFlashing(RgbColour colour) {
    ChangeColour(255, 255, 255);
    _flashPin = PinFromColour(colour);
    _flashStartMs = millis();
    _flashing = true;
  }

  void StopFlashing() {
    if (_flashing) {
      analogWrite(_flashPin, 255);
      _flashing = false;
    }
  }

  void Update() {
    if (!_flashing)
      return;

    const int CYCLETIME = 1000;
    const double HALFCYCLE = CYCLETIME / 2.0;

    auto cycleMs = (millis() - _flashStartMs) % CYCLETIME;
    auto colourValue = (cycleMs / HALFCYCLE) * 256.0;
    if (cycleMs > HALFCYCLE)
      colourValue = 512 - colourValue;

    analogWrite(_flashPin, 255 - max(min(colourValue, 255), 0));
  }
private:
  uint8_t PinFromColour(RgbColour colour) {
    switch (colour) {
      case RgbColour::Red:
        return _redPin;
      case RgbColour::Green:
        return _greenPin;
      case RgbColour::Blue:
        return _bluePin;
    }
  }
  void ChangeColour(int redVal, int greenVal, int blueVal) {
    _redVal = redVal;
    _greenVal = greenVal;
    _blueVal = blueVal;
    WritePinValues();
  }
  void WritePinValues() {
    analogWrite(_redPin, _redVal);
    analogWrite(_greenPin, _greenVal);
    analogWrite(_bluePin, _blueVal);
  }
};
#endif