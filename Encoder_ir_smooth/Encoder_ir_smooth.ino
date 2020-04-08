// RGB(W) Led strip controller
// Atte Juvonen 2020

// IR Library
#include <IRremote.h>

// EEPROM Library ( For permanent saving feature )
#include <EEPROM.h>

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
#define STEP 50

// Power state
bool power = false;...

// Are we saving a colour?
bool saving = false;


// ==============  Arrays etc. for storing data  ===============================

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

// Secondary save slot for indicating selected etc.
uint16_t saved2[3] = { 0, 0, 0 };

// For saving colours
typedef struct {
  uint16_t r;
  uint16_t g;
  uint16_t b;
} Colour;

Colour saved_colours[3] = {
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}
};

int ir_codes[] = { // When
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  101,  // UP
  102,  // DOWN
  103,  // LEFT
  104,  // RIGHT
  105,  // OK
  106,  // STAR
  107  // HASHTAG
};

int ir_hexcodes[] = {
  0xFF4AB5,
  0xFF6897,
  0xFF9867,
  0xFFB04F,
  0xFF30CF,
  0xFF18E7,
  0xFF7A85,
  0xFF10EF,
  0xFF38C7,
  0xFF5AA5,
  0xFF629D,
  0xFFA857,
  0xFF22DD,
  0xFFC23D,
  0xFF02FD,
  0xFF42BD,
  0xFF52AD
};


// ==============  IR Reciever  ================================================

// IR recieve pin
uint8_t RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);
decode_results results;


// ==============  SETUP  ======================================================

void setup() {
  // Random seed from analog noise
  randomSeed(analogRead(A5));

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
    Serial.println(results.value, HEX);
    irActions();
    irrecv.resume();
    updatePwm(100);
  }

  if (saving) {
    saveColour();
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
      new_percieved[colour] = 1000;
    } else {
      new_percieved[colour] = new_value;
    }
    power = true;
  } else {  // Decrease
    if (new_value < 0) {
      new_percieved[colour] = 0;
    } else {
      new_percieved[colour] = new_value;
    }
  }
}

// ==============  Decode Ir reciever values  ==================================

int decodeIrv2() {
  Serial.println("decodeIrv2.called");
  int value = results.value;

  for (uint8_t i = 0; i < 17; i++) {
    if (value == ir_hexcodes[i]) {
      Serial.print("decodeIrv2.returning ");
      Serial.println(ir_codes[i]);
      return ir_codes[i];
    }
  }

  return -1;
}

void irActions() {
  int key = decodeIrv2();
  if (key == 0) {
    saving = true;
  } else if (key >= 1 && key <= 9) {
    switchTo(key);
  } else if (key == 105) {
    changeColour();
  } else if (key == 101) {
    step(current_colour, STEP);
  } else if (key == 102) {
    step(current_colour, - STEP);
  } else if (key == 106) {
    randomColour();
  } else if (key == 107) {
    switchPwr();
  }
  // switch(results.value) {
  //   case 0x12758: // up
  //     // Serial.println("up");
  //     step(current_colour, STEP);
  //     break;
  //   case 0x2758: // up
  //     // Serial.println("up");
  //     step(current_colour, STEP);
  //     break;
  //   case 0x12759: // down
  //     // Serial.println("decodeIr.down");
  //     step(current_colour, - STEP);
  //     break;
  //   case 0x2759: // down
  //     // Serial.println("decideIr.down");
  //     step(current_colour, - STEP);
  //     break;
  //   case 0x1275C: // ok
  //     // Serial.println("ok");
  //     changeColour();
  //     break;
  //   case 0x275C: // ok
  //     // Serial.println("ok");
  //     changeColour();
  //     break;
  //   case 0x270C: // pwr
  //     //Serial.println("pwr");
  //     switchPwr();
  //     break;
  //   case 0x1270C: // pwr
  //     //Serial.println("pwr");
  //     switchPwr();
  //     break;
  //   case 0x27CB: // info
  //     randomColour();
  //     break;
  //   case 0x127CB: // info
  //     randomColour();
  //     break;
  //   case 0x2701: // 1
  //     switchTo(1);
  //     break;
  //   case 0x12701: // 1
  //     switchTo(1);
  //     break;
  //   case 0x2702: // 2
  //     switchTo(2);
  //     break;
  //   case 0x12702: // 2
  //     switchTo(2);
  //     break;
  //   case 0x2703: // 3
  //     switchTo(3);
  //     break;
  //   case 0x12703: // 3
  //     switchTo(3);
  //     break;
  //   case 0x2700: // 0
  //     saving = true;
  //     break;
  //   case 0x12700: // 0
  //     saving = true;
  //     break;
  //  }
  //
  //  return 11;
}


// ==============  Decode numbers for saving ===================================

uint8_t decodeSaving() {  // bulls**t function but no can do
  switch(results.value) {
    case 0x2701: // 1
      return 1;
    case 0x12701: // 1
      return 1;
    case 0x2702: // 2
      return 2;
    case 0x12702: // 2
      return 2;
    case 0x2703: // 3
      return 3;
    case 0x12703: // 3
      return 3;
    default:
      return 11;
  }
}


// ==============  Set random colour  ==========================================

void randomColour() {
  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = random(1001);
  }
  updatePwm(1000);
  power = true;
}


// ==============  Save current colour ========================================

void saveColour() {
  // 11 won't ever get pressed :D
  uint8_t pressed = 11;

  while (saving) {
    // Mark down the displayed colour
    for (uint8_t i = 0; i < 3; i++) {
      saved2[i] = percieved[i];
    }

    // Flash the colour slowly to indicate active saving mode
    for (uint8_t i = 0; i < 3; i++) {
      new_percieved[i] = percieved[i] / 2;
    }
    updatePwm(400);
    for (uint8_t i = 0; i < 3; i++) {
      new_percieved[i] = saved2[i];
    }
    updatePwm(400);

    // If a button is pressed
    if (irrecv.decode(&results)) {
      pressed = decodeIrv2();
      if (pressed >= 1 && pressed <= 9) {
        // Save the colour
        permanentSaveCurrent(pressed);
        // saved_colours[pressed - 1] = {percieved[0], percieved[1], percieved[2]};
        saving = false;
        confirmSaved();
      }
      irrecv.resume();
    }
  }
}


// ==============  Save current colour to EEPROM  ==============================

void permanentSaveCurrent(uint8_t buttonPressed) {
  int address = (buttonPressed - 1) * 3;
  EEPROM.write(address, percieved[0] / 1000.0 * 255.0);
  EEPROM.write(address + 1, percieved[1] / 1000.0 * 255.0);
  EEPROM.write(address + 2, percieved[2] / 1000.0 * 255.0);
}


// ==============  Confirm the saved colour  ===================================

void confirmSaved() {

  // Just flash the saved colour a couple times
  for (uint8_t i = 0; i < 3; i++) {
    saved2[i] = percieved[i];
  }

  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = percieved[i] / 2 ;
  }
  updatePwm(100);

  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = saved2[i];
  }
  updatePwm(100);

  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = percieved[i] / 2 ;
  }
  updatePwm(100);

  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = saved2[i];
  }
  updatePwm(100);
}


// ==============  Change the current colour  ==================================

void changeColour() {
  Serial.println("changeColour.switched!");
  if (current_colour + 1 > 2) {
    current_colour = 0;
  } else {
    current_colour = current_colour +1;
  }
  indicateSelected();
}


// ==============  Switch to a saved colour  ===================================

void switchTo(uint8_t pressed) {
  power = true;
  int address = (pressed - 1) * 3;
  new_percieved[0] = EEPROM.read(address) / 255.0 * 1000.0;
  new_percieved[1] = EEPROM.read(address + 1) / 255.0 * 1000.0;
  new_percieved[2] = EEPROM.read(address + 2) / 255.0 * 1000.0;
  updatePwm(1000);
}


// ==============  Indicate the selected colour  ===============================

void indicateSelected() {
  for (uint8_t i = 0; i < 3; i++) {
    saved2[i] = percieved[i];
  }

  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = 0;
  }
  updatePwm(200);

  new_percieved[current_colour] = 700;
  updatePwm(200);
  delay(100);
  new_percieved[current_colour] = 0;
  updatePwm(400);

  for (uint8_t i = 0; i < 3; i++) {
    new_percieved[i] = saved2[i];
  }
  updatePwm(400);
}

// ==============  Update PWM states  ==========================================

void updatePwm(uint16_t duration) {
  if (duration == 0) {  // Instant change, for encoders
    for (uint8_t i = 0; i < 3; i++) {
      pwm[i] = (255.0)/(pow(1000, 3)) * pow(new_percieved[i], 3);
      analogWrite(pwm_pins[i], pwm[i]);
      percieved[i] = new_percieved[i];
    }
  } else {  // Smoooooth change, for IR remote actions
    for (int t = 1; t < duration; t += 3) {
      for (uint8_t i = 0; i < 3; i++) {

        int diff = new_percieved[i] - percieved[i];

        int ramp_perc = (diff/2) - (diff/2) * cos((t * PI) / duration ) +
                        percieved[i];

        pwm[i] = (255.0)/(pow(1000, 3)) * pow(ramp_perc, 3);

        analogWrite(pwm_pins[i], pwm[i]);

      }
      delay(2);
    }

    for (uint8_t i = 0; i < 3; i++) {
      percieved[i] = new_percieved[i];
    }
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
      new_percieved[i] = 0;
    }
    power = false;
    updatePwm(1000);
  } else {
    for (uint8_t i = 0; i < 3; i++) {
      new_percieved[i] = saved[i];
    }
    power = true;
    updatePwm(800);
  }
}
