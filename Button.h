#ifndef Button_h
#define Button_h

enum class ButtonState { Pressed,
                         Released };


class Button {
private:
  unsigned long _buttonPressedMs{ 0 };
  unsigned long _buttonReleasedMs{ 0 };
  ButtonState _buttonState{ ButtonState::Released };
  uint8_t _pin;
  uint16_t _state = 0;
  unsigned long _lastChangeMs{ 0 };
public:
  Button() {}
  void Begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
  }
  ButtonState State() const {
    return _buttonState;
  }
  inline bool IsPressed() const {
    return _buttonState == ButtonState::Pressed;
  }
  void Update() {
    auto frameTimeMs = millis();
    _state = (_state << 1) | digitalRead(_pin) | 0xfe00;
    switch (_state & 0xff) {
      case 0xff:
        if (_buttonState != ButtonState::Pressed && frameTimeMs - _lastChangeMs > 5) {
          _buttonState = ButtonState::Pressed;
          _buttonPressedMs = frameTimeMs;
          _lastChangeMs = _buttonPressedMs;
        }
        break;
      case 0x00:
        if (_buttonState != ButtonState::Released && frameTimeMs - _lastChangeMs > 5) {
          _buttonState = ButtonState::Released;
          _buttonReleasedMs = frameTimeMs;
          _lastChangeMs = _buttonReleasedMs;
        }
        break;
      default:
        // bouncing
        break;
    }
  }
  // When button state is Released, returns time that the button
  // had been pressed (in ms).
  inline unsigned long PressedMs() const {
    return _buttonReleasedMs - _buttonPressedMs;
  }
  // When button state is Pressed, returns time that the button
  // had been released (in ms).
  inline unsigned long ReleasedMs() const {
    return _buttonPressedMs - _buttonReleasedMs;
  }
  inline unsigned long LastChangedMs() const {
    return _lastChangeMs;
  }
};

#endif