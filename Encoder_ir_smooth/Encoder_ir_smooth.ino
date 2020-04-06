// RGB(W) Led strip controller
// Atte Juvonen 2020

// IR Library
#include <IRremote.h>

// Rotary encoder clock pins
#define r_clk  digitalRead(A0)
#define g_clk  digitalRead(4)
#define b_clk  digitalRead(7)

// Used pwm pins
uint8_t pwm_pins[3] = { 10, 5, 6 };

// Rotary encoder data pins
#define r_dt digitalRead(8)
#define g_dt digitalRead(9)
#define b_dt digitalRead(11)

// How much to increment the percieved brightness ( 0 - 1000)
#define STEP 100

// Power state
bool power = false;


// ==============  Arrays for storing data  ====================================

// Clock signal states
bool clk_state[3] = { 0, 0, 0 };

// Previous clock states
bool clk_last[3] = { r_clk, g_clk, b_clk };

// Data signal states
bool dt[3] = { 0, 0, 0 };

// Percieved brightness ( 0 - 1000 )
uint16_t percieved[3] = { 0, 0, 0 };

// Desired percieved brightness ( 0 - 1000 )
uint16_t new_percieved[3] = { 0, 0, 0 };

// PWM values  ( 0 - 255 )
uint8_t pwm[3] = { 0, 0, 0 };

// Which colour is being adjusted (default = red)
uint8_t current_colour = 0;

// Save the colours for powering off
uint16_t saved[3] = { 0, 0, 0 };


// ==============  IR Reciever  ================================================

// IR recieve pin
uint8_t RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);
decode_results results;


// ==============  SETUP  ======================================================

void setup() {
  pinMode(pwm_pins[0], OUTPUT);
  pinMode(pwm_pins[1], OUTPUT);
  pinMode(pwm_pins[2], OUTPUT);

  pinMode(r_clk, INPUT);
  pinMode(g_clk, INPUT);
  pinMode(b_clk, INPUT);

  pinMode(r_dt, INPUT);
  pinMode(g_dt, INPUT);
  pinMode(b_dt, INPUT);

  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}


// ==============  LOOP  =======================================================

void loop() {

  // Current clock states
  clk_state[0] = r_clk;
  clk_state[1] = g_clk;
  clk_state[2] = b_clk;

  // Current data states
  dt[0] = r_dt;
  dt[1] = g_dt;
  dt[2] = b_dt;

  // Decode rotary encoder data
  for (uint8_t i = 0; i < 3; i++) {
    if (clk_state[i] != clk_last[i]) {
      if (dt[i] != clk_state[i]) {
        step(i, STEP);
      } else {
        step(i, -STEP);
      }
    }
  }

  // Mark down clock states
  for (int i = 0; i <= 2; i++) {
    clk_last[i] = clk_state[i];
  }

  // Delay for stability
  delay(2);

  // IR reciever
  if (irrecv.decode(&results)) {
    // Serial.println(results.value, HEX);
    int value = results.value;
    decodeIr(value);
    irrecv.resume();
  }

  // Update pwm states
  updatePwm(0);

  // Print brightnesses to Serial
  // printPwm();
}

// ==============  Step percieved brightness up or down  =======================

void step(uint8_t colour, int step) {
  int new_value = percieved[colour] + step;

  if (step > 0) {  // Increase
    if (new_value > 1000) {
      percieved[colour] = 1000;
    } else {
      percieved[colour] = new_value;
    }
    power = true;
  } else {  // Decrease
    if (new_value < 0) {
      percieved[colour] = 0;
    } else {
      percieved[colour] = new_value;
    }
  }
}

// ==============  Decode Ir reciever values  ==================================

void decodeIr(int &value) {
  switch(results.value) {
    case 0x12758:
      // Serial.println("up");
      step(current_colour, STEP);
      break;
    case 0x2758:
      // Serial.println("up");
      step(current_colour, STEP);
      break;
    case 0x12759:
      // Serial.println("decodeIr.down");
      step(current_colour, - STEP);
      break;
    case 0x2759:
      // Serial.println("decideIr.down");
      step(current_colour, - STEP);
      break;
    case 0x1275C:
      // Serial.println("ok");
      changeColour();
      break;
    case 0x275C:
      // Serial.println("ok");
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
}

// ==============  Change the current colour  ==================================

void changeColour() {
  Serial.println("changeColour.switched!");
  if (current_colour + 1 > 2) {
    current_colour = 0;
  } else {
    current_colour = current_colour +1;
  }
}


// ==============  Update PWM states ( no ramp )  ==============================

void updatePwm(uint16_t duration) {
  if (duration == 0) {
    for (uint8_t i = 0; i < 3; i++) {
      pwm[i] = (255.0)/(pow(1000, 3)) * pow(percieved[i], 3);
      analogWrite(pwm_pins[i], pwm[i]);
    }
  } else {

  }
}


// ==============  Plot pwm states  ============================================

void printPwm() {
  Serial.print("R: "); Serial.print(pwm[0]); Serial.print("  ");
  Serial.print("G: "); Serial.print(pwm[1]); Serial.print("  ");
  Serial.print("B: "); Serial.print(pwm[2]); Serial.print("  ");
  Serial.println();
}


// ==============  Switch power on or off  =====================================

void switchPwr() {
  if (power) {
    for (uint8_t i = 0; i < 3; i++) {
      saved[i] = percieved[i];
    }

    for (uint8_t i = 0; i < 3; i++) {
      percieved[i] = 0;
    }
    power = false;
  } else {
    for (uint8_t i = 0; i < 3; i++) {
      percieved[i] = saved[i];
    }
    power = true;
  }
}
