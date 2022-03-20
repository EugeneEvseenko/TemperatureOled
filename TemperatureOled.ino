/* 
    GitHub: https://github.com/EugeneEvseenko/TemperatureOled
    Eugene Evseenko, eugene@evseenko.kz
    https://evseenko.kz/
    MIT License
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <GyverOLED.h>
#include <TimerMs.h>
#include <EncButton.h>

#define ONE_WIRE_BUS 12 // пин с ds18b20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
GyverOLED<SSD1306_128x64> oled;
TimerMs tmr(5 * 60 * 1000, 1, 1); // таймер дисплея
TimerMs tempTmr(1000, 1); // таймер считывания температуры, чтение каждую секунду
EncButton<EB_TICK, 14> btn; // кнопка пробуждения
float temperature;

void setup() {
  Serial.begin(115200);
  tmr.setTimerMode();
  tempTmr.setPeriodMode();
  sensor.begin();
  sensor.setResolution(11); // задержка считывания около 400 мс, при 12бит - около 800 мс
  oled.init();
  PrintTemperature();
}

void loop() {
  btn.tick(); tmr.tick(); tempTmr.tick();

  if (btn.click()) { // пробудить экран / перезапустить таймеры
    PrintTemperature();
    //Serial.println("click");
    tempTmr.start();
    oled.setPower(true);
    tmr.start();
  }
  if (tempTmr.ready() && tmr.active()) {
    PrintTemperature();
  }
  if (tmr.ready()) {
    tempTmr.stop();
    //Serial.println("turn off");
    oled.setPower(false);
  }
  // Serial.print(tmr.active() ? " active" : " not active");Serial.print(tmr.elapsed() ? " elapsed | " : " not elapsed | ");
  // Serial.print(tempTmr.active() ? " active" : " not active");Serial.println(tempTmr.elapsed() ? " elapsed" : " not elapsed");
}

void PrintTemperature() {
  sensor.requestTemperatures(); // запрос температуры
  temperature = sensor.getTempCByIndex(0); // чтение с первого и единственного датчика
  //temperature = random(125); // для проверки
  oled.clear();
  oled.home();
  oled.fastLineH(26, 0, 127);
  oled.fastLineH(27, 0, 127);

  oled.setScale(3);
  String text = String(temperature, 1);
  oled.setCursorXY(CalcCenter(text, 3, true), 0);
  oled.println(text);

  oled.setScale(2);
  text = GetState(temperature);
  oled.setCursorXY(CalcCenter(text, 2, false), 34);
  oled.println(text);

  //oled.fastLineH(54, 0, 127);oled.fastLineH(55, 0, 127);
  oled.setScale(1);
  oled.setCursor(0, 7);
  if (text == "МАЛО") {
    oled.invertText(true);
    oled.fastLineH(54, 0, 42);
    oled.fastLineH(55, 0, 42);
  }
  oled.setCursor(0, 7);
  oled.print("(<78.3)");
  oled.invertText(false);

  if (text == "НОРМАЛЬНО") {
    oled.invertText(true);
    oled.fastLineH(54, 43, 96);
    oled.fastLineH(55, 43, 96);
  }
  oled.setCursor(43, 7);
  oled.print("(78.3-94)");
  oled.invertText(false);

  if (text == "МНОГО") {
    oled.invertText(true);
    oled.fastLineH(54, 97, 127);
    oled.fastLineH(55, 97, 127);
  }
  oled.setCursor(97, 7);
  oled.println("(>94)");
  oled.invertText(false);
  oled.update();
}

String GetState(float temp) {
  if (temp < 78.3) {
    return "МАЛО";
  } else if ( temp >= 78.3 && temp <= 94.0) {
    return "НОРМАЛЬНО";
  } else {
    return "МНОГО";
  }
}

int CalcCenter(String text, int scale, bool mode) {
  int CharOnScreen = scale == 2 ? 11.0 : 7.0; // костыль, русские символы занимают в два раза больше места
  float OneCharSize = 127.0 / CharOnScreen; // сколько места занимает один символ на дисплее
  float AllCharSize = OneCharSize * (text.length() / (mode ? 1.0 : 2.0)); // сколько занимают места все символы
  float left = 127.0 / 2.0; float right = AllCharSize / 2;
  int out = left - right;
  return out; // координата по оси Х в центре дисплея
}