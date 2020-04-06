#define datapin digitalRead(13)
#define clockpin digitalRead(12)

int clock_last_state;
int clock_state;
int r_brightness;

void setup() {
  pinMode(3, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  Serial.begin(9600);
  int clock_last_state = clockpin;
  r_brightness = 0;
  

}

void loop() {
  //sweepBrightness(255, 500, 2000, 4);
}

/*
unsigned int brightness(int value, int duration, int degree) {
  long int a;
  a = pow(duration, degree);
  float constant = 255.0/a;
  float b = pow(value, degree) * constant;
  Serial.println(b);
  Serial.println((unsigned int)b);

  // Serial.write("brightness trying to return ");
  // Serial.println(a);
  return (unsigned int)b;
}
*/

void sweepBrightness(float value, int duration1, int duration2, int degree) {
  uint64_t a1 = pow(duration1, degree);
  float constant1 = value/a1;

  uint64_t a2 = pow(duration2, degree);
  float constant2 = value/a2;


  analogWrite(3, 0);
  for(int i = 1; i <= duration1; i++) {
    float b = pow(i, degree) * constant1;
    analogWrite(3, b);
    delay(1);
  }

  delay(400);

  for(int i = duration2; i >= 0; i--) {
    float b = pow(i, degree) * constant2;
    analogWrite(3, b);
    delay(1);
  }

  delay(100);
}
