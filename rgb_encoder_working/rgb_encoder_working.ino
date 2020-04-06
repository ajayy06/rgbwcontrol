#include <IRremote.h>

#define RED 0
#define GREEN 1
#define BLUE 2

#define r_pwm 10
#define g_pwm 5
#define b_pwm 6

#define r_clk  digitalRead(A0)
#define g_clk  digitalRead(4)
#define b_clk  digitalRead(7)

#define r_dt digitalRead(8)
#define g_dt digitalRead(9)
#define b_dt digitalRead(11)

#define STEP 10

int clk_state[3];
int dt[3];
int clk_last[3] = { r_clk, g_clk, b_clk };
int percentage[3] = { 0, 0, 0 };
int brightness[3] = { 0, 0, 0 };
char colours[3] = { 'R', 'G', 'B' };
uint8_t pwm_pins[3] = { 10, 5, 6 };
int colour_to_adjust = 0;

bool power = true;
int saved_colours[3] = {0, 0, 0};

int RECV_PIN = 2;
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
  updateBrightness();
  Serial.println(brightness[1]);
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
        changePercentage(i, true);
      } else {
        Serial.println("---");
        Serial.println(colours[i]);
        changePercentage(i, false);
      }

      if (not power) {
        power = true;
      }
      
    }
  }

  for (int i = 0; i <= 2; i++) {
    clk_last[i] = clk_state[i];
  }

  delay(2);



    if (irrecv.decode(&results)) {
      Serial.println(results.value, HEX);

     switch(results.value) {
      case 0x12758:
        Serial.println("up");
        //changePercentage(colour_to_adjust, true);
        changeColours(100, 0, 0, 1000);
        break;
      case 0x2758:
        Serial.println("up");
        //changePercentage(colour_to_adjust, true);
        changeColours(100, 0, 0, 1000);
        break;
      case 0x12759:
        Serial.println("down");
        //changePercentage(colour_to_adjust, false);
        changeColours(-100, 0, 0, 1000);
        break;
      case 0x2759:
        Serial.println("down");
        //changePercentage(colour_to_adjust, false);
        changeColours(-100, 0, 0, 1000);
        break;
      case 0x1275C:
        Serial.println("ok");
        changeColour();
        break;
      case 0x275C:
        Serial.println("ok");
        changeColour();
        break;
      case 0x270C:
        //Serial.println("pwr");
        switchPwr();
        break;
      case 0x1270C:
        //Serial.println("pwr");
        switchPwr();
        break;
     }

     irrecv.resume(); // Receive the next value
    }
}

void updateBrightness() {
  for (int i = 0; i <= 2; i++) {
    brightness[i] = 51.0/200000.0 * pow(percentage[i], 3);
    analogWrite(pwm_pins[i], brightness[i]);
  }
}


void changePercentage(int colour, bool increase) {
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

void changeColours(const int r, const int g, const int b, int duration) {
  int orig_percentages[3] = {percentage[0] * 10, percentage[1] * 10, percentage[2] * 10};
  int change[3] = {r * 10, g * 10, b * 10};
  int new_percentages[3] = {0, 0, 0};

  for (int i = 0; i < 3; i++) {
    if (orig_percentages[i] + change[i] > 1000) {
      new_percentages[i] = 1000;
    } else if (orig_percentages[i] + change[i] < 0) {
      new_percentages[i] = 0;
    } else {
      new_percentages[i] = orig_percentages[i] + change[i];
    }
  }
  int pwm[3] = {0, 0, 0};

  for (int t = 0; t < duration; t+= 3) {
    for (int j = 0; j < 3; j++) {
      int percieved_lvl = (new_percentages[j] - orig_percentages[j])/(2) -
      (new_percentages[j] - orig_percentages[j])/(2) * cos((t * PI)/(duration)) + orig_percentages[j];

      int pwm_lvl = ((new_percentages[j] - orig_percentages[j])/(pow(new_percentages[j] - orig_percentages[j], 3)) *
      pow(percieved_lvl - min(new_percentages[j], orig_percentages[j]), 3) + min(new_percentages[j], orig_percentages[j])) * (255)/(1000);

      brightness[j] = pwm_lvl;
      percentage[j] = percieved_lvl / 10;
      analogWrite(pwm_pins[j], pwm_lvl);
    }
    //Serial.println(percentage[1]);
    Serial.println(percentage[1]);
    delay(2);
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
  analogWrite(pwm_pins[colour], 40);
  delay(100);
  analogWrite(pwm_pins[colour], 0);
  delay(100);
  analogWrite(pwm_pins[colour], 40);
  delay(200);
  for (int i = 0; i <= 2; i++) {
    brightness[i] = 51.0/200000.0 * pow(percentage[i], 3);
    analogWrite(pwm_pins[i], brightness[i]);
  }
}

void switchPwr() {
  if (power) {
    for (int i = 0; i < 3; i++) {
      saved_colours[i] = percentage[i];
    }
    changeColours(-percentage[0], -percentage[1], -percentage[2], 1000);
    power = false;
  } else {
    changeColours(saved_colours[0], saved_colours[1], saved_colours[2], 800);
    power = true;
  }
}
