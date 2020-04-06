#include <IRremote.h>

#define RED 0
#define GREEN 1
#define BLUE 2

#define r_pwm 10
#define g_pwm 5
#define b_pwm 6

#define r_clk  digitalRead(2)
#define g_clk  digitalRead(4)
#define b_clk  digitalRead(7)

#define r_dt digitalRead(8)
#define g_dt digitalRead(9)
#define b_dt digitalRead(11)

#define STEP 5

int clk_state[3];
int dt[3];
int clk_last[3] = { r_clk, g_clk, b_clk };
int percentage[3] = { 0, 0, 0 };
int brightness[3] = { 0, 0, 0 };
char colours[3] = { 'R', 'G', 'B' };
uint8_t pwm_pins[3] = { 10, 5, 6 };
int colour_to_adjust = 0;

int RECV_PIN = A5;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  pinMode(r_pwm, OUTPUT);
  pinMode(g_pwm, OUTPUT);
  pinMode(b_pwm, OUTPUT);
  
  pinMode(r_clk, INPUT);
  pinMode(g_clk, INPUT);
  pinMode(b_clk, INPUT);

  pinMode(r_dt, INPUT);
  pinMode(g_dt, INPUT);
  pinMode(b_dt, INPUT);
  
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  clk_state[RED] = r_clk;
  clk_state[GREEN] = g_clk;
  clk_state[BLUE] = b_clk;
  
  dt[RED] = r_dt;
  dt[GREEN] = g_dt;
  dt[BLUE] = b_dt;

  for (int i = 0; i <= 2; i++) {
    if (clk_state[i] != clk_last[i]) {
      if (dt[i] != clk_state[i]) {
        Serial.println("+++");
        Serial.println(colours[i]);
        changeBrightness(i, true);
      } else {
        Serial.println("---");
        Serial.println(colours[i]);
        changeBrightness(i, false);
      }
    }
  }

  for (int i = 0; i <= 2; i++) {
    clk_last[i] = clk_state[i];
  }

  for (int i = 0; i <= 2; i++) {
    brightness[i] = 51.0/200000.0 * pow(percentage[i], 3);
    analogWrite(pwm_pins[i], brightness[i]);
  }
  // r_brightness = 51.0/200000.0 * pow(r_percentage, 3);
  // analogWrite(3, r_brightness);
  delay(2);

    if (irrecv.decode(&results)) {
     // Serial.println(results.value, HEX);

     switch(results.value) {
      case 0x12758:
        Serial.println("up");
        changeBrightness(colour_to_adjust, true);
        break;
      case 0x2758:
        Serial.println("up");
        changeBrightness(colour_to_adjust, true);
        break;
      case 0x12759:
        Serial.println("down");
        changeBrightness(colour_to_adjust, false);
        break;
      case 0x2759:
        Serial.println("down");
        changeBrightness(colour_to_adjust, false);
        break;
      case 0x1275C:
        Serial.println("ok");
        changeColour();
        break;
      case 0x275C:
        Serial.println("ok");
        changeColour();
        break;
     }
     
     irrecv.resume(); // Receive the next value
    }
}


void changeBrightness(int colour, bool increase) {
  int new_percentage;
  Serial.println(colour);
  switch (increase) {
    case true:
      new_percentage = percentage[colour] + STEP;
      if (new_percentage > 100) {
        percentage[colour] = 100;
      } else {
        percentage[colour] = new_percentage;
      }
      break;
    case false:
      new_percentage = percentage[colour] - STEP;
      if (new_percentage < 0) {
        percentage[colour] = 0;
      } else {
        percentage[colour] = new_percentage;
      }
      break;
  }
}

void changeColour() {
  Serial.println("switched!");
  if (colour_to_adjust + 1 > 2) {
    colour_to_adjust = 0;
  } else {
    colour_to_adjust = colour_to_adjust +1;
  }

  indicateSelected(colour_to_adjust);
}

void indicateSelected(int colour) {

  for (int i = 0; i <= 2; i++) {
    analogWrite(pwm_pins[i], 0);
  }

  delay(100);
  analogWrite(pwm_pins[colour], 255);
  delay(100);
  analogWrite(pwm_pins[colour], 0);
  delay(100);
  analogWrite(pwm_pins[colour], 255);
  delay(200);
  for (int i = 0; i <= 2; i++) {
    brightness[i] = 51.0/200000.0 * pow(percentage[i], 3);
    analogWrite(pwm_pins[i], brightness[i]);
  }
}
