#include "LowPower.h"
#include <EEPROM.h>
#include "TonePlayer.h"
#include "Blinker.h"
#include "RgbLed.h"
#include "Button.h"

constexpr uint8_t PIN_TESTBUTTON{ 2 };
constexpr uint8_t PIN_PIR{ 3 };
constexpr uint8_t PIN_BLUE{ 9 };
constexpr uint8_t PIN_GREEN{ 10 };
constexpr uint8_t PIN_RED{ 6 };
constexpr uint8_t PIN_BUZZER{ 5 };
constexpr uint8_t PIN_MORSE{ 4 };

constexpr int EEPROM_CURRENT_ALARM = 0;

constexpr Tone CloseEncounters[] = {
  { 932, 250 }, { 0, 30 }, { 1047, 250 }, { 0, 30 }, { 831, 250 }, { 0, 200 }, { 415, 500 }, { 0, 100 }, { 622, 2000 }
};
constexpr Tone Alarm2[] = {
  { 1000, 500 }, { 600, 500 }
};
constexpr Tone Alarm3[] = {
  { 100, 200 }, { 200, 200 }, { 500, 500 }, { 1000, 500 }, { 0, 500 }
};
constexpr Tone Alarm4[] = {
  { 1000, 200 }, { 800, 200 }
};
constexpr Tone Alarm5[] = {
  { 1200, 200 }, { 0, 200 }
};
struct Alarm {
  Tone* Tones;
  int Size;
};

constexpr Alarm Alarms[] = {
  { CloseEncounters, sizeof(CloseEncounters) / sizeof(Tone) },
  { Alarm2, sizeof(Alarm2) / sizeof(Tone) },
  { Alarm3, sizeof(Alarm3) / sizeof(Tone) },
  { Alarm4, sizeof(Alarm4) / sizeof(Tone) },
  { Alarm5, sizeof(Alarm5) / sizeof(Tone) },
};

constexpr unsigned long BlinkPattern[] = { 200, 100, 200, 800 };

const RgbLed led{};
const Blinker blinker{};
const TonePlayer tonePlayer{};
const Button button{};

bool alarm{ false };
int currentAlarm{ 1 };

volatile unsigned long lastInterruptMs{ 0 };

// Interrupt Service Routines (ISR)
void testButtonChanged() {
  lastInterruptMs = millis();
}

volatile bool motionDetected{ false };
volatile bool motionDetectionEnded{ false };
volatile unsigned long motionDetectedMs{ 0 };

void pirStatusChanged() {
  lastInterruptMs = millis();
  if (digitalRead(PIN_PIR) == HIGH) {
    motionDetected = true;
    motionDetectedMs = millis();
  } else {
    motionDetectionEnded = true;
    motionDetectedMs = 0;
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Started.");

  currentAlarm = EEPROM.read(EEPROM_CURRENT_ALARM);
  Serial.print("Current alarm: ");
  Serial.println(currentAlarm);
  if (currentAlarm > sizeof(Alarms) / sizeof(Alarm))
    currentAlarm = 0;

  tonePlayer.Begin(PIN_BUZZER);
  blinker.Begin(LED_BUILTIN, LOW);
  led.Begin(PIN_RED, PIN_GREEN, PIN_BLUE);
  button.Begin(PIN_TESTBUTTON);

  blinker.Start(BlinkPattern, sizeof(BlinkPattern) / sizeof(unsigned long));

  attachInterrupt(digitalPinToInterrupt(PIN_TESTBUTTON), testButtonChanged, CHANGE);

  pinMode(PIN_PIR, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_PIR), pirStatusChanged, CHANGE);

  pinMode(PIN_MORSE, OUTPUT);
}

// enum class Mode { Normal,
//                   Configure };
// Mode mode{ Mode::Normal };

void handleButtonInput(unsigned long frameTimeMs, void (*gotWordHandler)(String&)) {
  static ButtonState bs{ ButtonState::Released };
  static String word{};

  if (word.length() > 0 && button.State() == ButtonState::Released && frameTimeMs - button.LastChangedMs() > 500) {
    Serial.println(word.c_str());

    gotWordHandler(word);

    word = "";
  }

  if (button.State() != bs) {

    unsigned long duration;

    String msg{};

    if (button.State() == ButtonState::Pressed) {
      duration = button.ReleasedMs();

      //tone(PIN_MORSE, 700);
      digitalWrite(PIN_MORSE, HIGH);
      //analogWrite(PIN_MORSE, 255);
    } else {
      duration = button.PressedMs();
      //noTone(PIN_MORSE);
      digitalWrite(PIN_MORSE, LOW);

      if (duration < 120) {
        msg += "DOT ";
        word += ".";
      } else {
        msg += "DASH ";
        word += "-";
      }
    }

    msg += duration;
    Serial.println(msg.c_str());

    bs = button.State();
  }
}

unsigned long iteration{ 0 };
unsigned long appStartMs{ 0 };

void changeAlarm(int index) {
  currentAlarm = index;
  EEPROM.write(EEPROM_CURRENT_ALARM, currentAlarm);
  tonePlayer.Play(Alarms[currentAlarm].Tones, Alarms[currentAlarm].Size, true);
}

void gotWordHandler(String& word) {
  if (word == ".----") {
    changeAlarm(0);
  } else if (word == "..---") {
    changeAlarm(1);
  } else if (word == "...--") {
    changeAlarm(2);
  } else if (word == "....-") {
    changeAlarm(3);
  } else if (word == ".....") {
    changeAlarm(4);
  } else if (word == "-----") {
    tonePlayer.Stop();
  }

  else if (word == ".-") {
    Serial.print("fps: ");
    Serial.println(iteration / (millis() / 1000.0));
  }
}

void loop() {
  auto frameTimeMs{ millis() };
  iteration++;
  if (appStartMs == 0) appStartMs = frameTimeMs;

  button.Update();

  handleButtonInput(frameTimeMs, gotWordHandler);

  //   if (button.IsPressed() && button.PressDuration(frameTimeMs) > 500) {
  //     if (!tonePlayer.IsPlaying())
  //       tonePlayer.Play(CloseEncounters, sizeof(CloseEncounters) / sizeof(Tone));
  //   }

  if (motionDetected) {
    led.StartFlashing(RgbColour::Blue);
    motionDetected = false;
  }
  if (motionDetectionEnded) {
    led.StopFlashing();
    motionDetectionEnded = false;
  }
  led.Update();

  blinker.Update();


  if (motionDetectedMs > 0 && frameTimeMs - motionDetectedMs > 10000) {
    if (!alarm) {
      led.ChangeFlashColour(RgbColour::Red);
      if (!tonePlayer.IsPlaying())
        //tonePlayer.Play(Alarm, sizeof(Alarm) / sizeof(Tone), true);
        tonePlayer.Play(Alarms[currentAlarm].Tones, Alarms[currentAlarm].Size, true);
      alarm = true;
    }
  } else
    alarm = false;


  tonePlayer.Update();

  if (frameTimeMs - lastInterruptMs > 12000 && !tonePlayer.IsPlaying()) {
    Serial.println("Going to sleep.");
    Serial.flush();
    blinker.Stop();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    Serial.println("Waking up.");
    blinker.Resume();
  }

  //delay(5);
}