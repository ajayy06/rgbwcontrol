#define datapin digitalRead(13)
#define clockpin digitalRead(12)
#define STEP 5

int clock_last_state;
int clock_state;
int r_percentage;
int r_brightness;

void setup() {
  pinMode(3, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  Serial.begin(9600);
  int clock_last_state = clockpin;
  r_percentage = 0;
  r_brightness = 0;
  

}

void loop() {
  //sweepBrightness(255, 500, 2000, 4);
  clock_state = clockpin;

  if (clock_state != clock_last_state) {
    if (datapin != clock_state) {
      Serial.println("Increase");
      changeBrightness(true);
    } else {
      Serial.println("Decrease");
      changeBrightness(false);
    }
  }

  clock_last_state = clock_state;
  r_brightness = 51.0/200000.0 * pow(r_percentage, 3);
  analogWrite(3, r_brightness);
  delay(2);
}

void changeBrightness(bool increase) {
  int new_percentage;
  switch (increase) {
    case true:
      new_percentage = r_percentage + STEP;
      if (new_percentage > 100) {
        r_percentage = 100;
      } else {
        r_percentage = new_percentage;
      }
      break;
    case false:
      new_percentage = r_percentage - STEP;
      if (new_percentage < 0) {
        r_percentage = 0;
      } else {
        r_percentage = new_percentage;
      }
      break;
  }
}
