#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET 11
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <DHT.h>

#define DHTTYPE DHT11

#define DHTPIN 2

#define MOTORSPEED 11

#define DIRA 8
#define DIRB 7

#define DELAY_OFF (1.5 * 1000)
#define DELAY_MEDIUM (3 * 1000)
#define DELAY_FAST (5 * 1000)

#define BUZZER 3
#define PWM_FILL 112

#define GREEN 6
#define YELLOW 5
#define RED 4

DHT dht(DHTPIN, DHTTYPE, 6);

#include "RTClib.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday"};

struct status {
        float temp;
        float humi;
};

void init_leds(void) {
        pinMode(GREEN, OUTPUT);
        pinMode(YELLOW, OUTPUT);
        pinMode(RED, OUTPUT);

        digitalWrite(GREEN, LOW);
        digitalWrite(YELLOW, LOW);
        digitalWrite(RED, LOW);
}

void init_motor(void) {
        pinMode(MOTORSPEED, OUTPUT);
        analogWrite(MOTORSPEED, 0);
}

void init_buzzer(void) {
        pinMode(BUZZER, OUTPUT);
        analogWrite(BUZZER, 0);
}

void init_rtc(void) {
        if (!rtc.begin()) {
                Serial.println("Couldn't find RTC");
        }

        if (!rtc.initialized() || rtc.lostPower()) {
                Serial.println("RTC is NOT initialized, let's set the time!");
                rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        rtc.start();

        float drift = 43; // seconds plus or minus over oservation period - set
                          // to 0 to cancel previous calibration.
        float period_sec =
            (7 * 86400); // total obsevation period in seconds (86400 = seconds
                         // in 1 day:  7 days = (7 * 86400) seconds )
        float deviation_ppm = (drift / period_sec *
                               1000000); //  deviation in parts per million (μs)
        float drift_unit = 4.34; // use with offset mode PCF8523_TwoHours
        int offset = round(deviation_ppm / drift_unit);

        Serial.print("Offset is ");
        Serial.println(offset); // Print to control offset
}

void setup() {
        Serial.begin(57600);

        init_leds();
        init_motor();

        init_buzzer();

        dht.begin();

        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
                Serial.println(F("SSD1306 allocation failed"));
        }

        display.display();
        delay(2000);
        display.clearDisplay();

        init_rtc();
}

void turn_off(void) {
        analogWrite(MOTORSPEED, 0);

        digitalWrite(DIRA, LOW);
        digitalWrite(DIRB, LOW);

        digitalWrite(GREEN, LOW);
        digitalWrite(YELLOW, LOW);
        digitalWrite(RED, HIGH);

        analogWrite(BUZZER, PWM_FILL);
        delay(DELAY_OFF);
        analogWrite(BUZZER, 0);
}

void turn_medium(void) {
        analogWrite(MOTORSPEED, 100);

        digitalWrite(DIRA, HIGH);
        digitalWrite(DIRB, LOW);

        digitalWrite(GREEN, LOW);
        digitalWrite(YELLOW, HIGH);
        digitalWrite(RED, LOW);

        delay(DELAY_MEDIUM);
}

void turn_fast(void) {
        analogWrite(MOTORSPEED, 200);

        digitalWrite(DIRA, HIGH);
        digitalWrite(DIRB, LOW);

        digitalWrite(GREEN, HIGH);
        digitalWrite(YELLOW, LOW);
        digitalWrite(RED, LOW);

        delay(DELAY_FAST);
}

status get_status(void) {
        float temp = dht.readTemperature();
        float humi = dht.readHumidity();

        Serial.print("Temperature = ");
        Serial.println(temp);
        Serial.print("Humidity = ");
        Serial.println(humi);

        display.clearDisplay();

        display.setTextSize(1);              // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); // Draw white text
        display.setCursor(0, 0);             // Start at top-left corner

        display.print(F("Temp = "));
        display.println(temp);
        display.print(F("Humi = "));
        display.println(humi);

        DateTime now = rtc.now();
        display.print(now.year());
        display.print('/');
        display.print(now.month());
        display.print('/');
        display.print(now.day());
        display.print(' ');
        display.print(daysOfTheWeek[now.dayOfTheWeek()]);
        display.println();
        display.print(now.hour());
        display.print(':');
        display.print(now.minute());
        display.print(':');
        display.print(now.second());
        display.println();

        display.display();

        return {temp, humi};
}

void loop() {
        status s = get_status();

        if (s.temp > 40) {
                turn_off();
        } else if (s.temp > 30) {
                turn_medium();
        } else {
                turn_fast();
        }
}
