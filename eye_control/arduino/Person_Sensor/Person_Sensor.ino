#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 140  // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 520  // this is the 'maximum' pulse length count (out of 4096)


unsigned long currentBlinkMillis = 0;
unsigned long previousBlinkMillis = 0;          // will store last time number changed
long blinkInterval = random(5000, 1200);  // period at which to change number (in milliseconds)
int blinkNumber = random(1, 2);

unsigned long currentSleepMillis = 0;
unsigned long previousSleepMillis = 0;
const long sleepInterval = 20000;

unsigned long loop_number = 0;

enum States {
  DORMANT,  // eyelids are closed, eyes are not moving
  AWAKE     // eyelids are open, eyes are tracking faces
};

States State;

// our servo # counter
uint8_t servonum = 0;

int xval;
int yval;
int trimval;
int current_xval;
int prev_xval = 512;

int lexpulse;
int rexpulse;

int leypulse;
int reypulse;

int uplidpulse;
int lolidpulse;
int altuplidpulse;
int altlolidpulse;

int sensorValue = 0;
int outputValue = 0;
int switchval = 0;
int loopNumber = 0;

int ledPin = 3;
int sensorPin = A6;
unsigned long awakeTime = 30000;


void setup() {

  // You need to make sure you call Wire.begin() in setup, or the I2C access
  // below will fail.
  Wire.begin();
  pinMode(2, INPUT);
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  State = DORMANT;

  trimval = 500;  // this sets how wide the eyelids are positioned (higher number = wider eyes)
  trimval = map(trimval, 320, 580, -40, 40);
  uplidpulse = map(yval, 0, 1023, 400, 280);
  uplidpulse -= (trimval - 40);
  uplidpulse = constrain(uplidpulse, 280, 400);
  altuplidpulse = 680 - uplidpulse;

  lolidpulse = map(yval, 0, 1023, 410, 280);
  lolidpulse += (trimval / 2);
  lolidpulse = constrain(lolidpulse, 280, 400);
  altlolidpulse = 680 - lolidpulse;

  // Power up sequence to test eyes
  closeEyes();
  delay(3000);
  wakeup();
  delay(1000);
  driftoff();
  delay(1000);

  Serial.println("setup complete");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');  // read until newline
    command.trim();  // remove any whitespace or newline chars

    if (command.equalsIgnoreCase("wakeup")) {
      wakeup();
    }
    if (command.equalsIgnoreCase("sleep")) {
      driftoff();
    }
  }

  // MAIN LOOP ACTIONS WHEN AWAKE
  if (State == AWAKE) {
    unsigned long now = millis();

    // --- Random blink control ---
    if (now - previousBlinkMillis >= blinkInterval) {
      previousBlinkMillis = now;
      blink();
      // pick new random interval for next blink
      blinkInterval = random(2000, 6000);  
    }

    // --- Random look-around control ---
    if (now - loop_number >= 2500) {  // every ~2.5s pick a new glance
      loop_number = now;

      int newX = random(220, 440);  // horizontal glance range
      //int newY = random(280, 500);  // vertical glance range

      pwm.setPWM(0, 0, newX);  // left/right eyes
      //pwm.setPWM(1, 0, newY);

      delay(300);  // short pause for natural movement
    }
  }
}


void blink() {  // script to execute one blink

  trimval = 550;  // this sets how wide the eyelids are positioned (higher number = wider eyes)
  trimval = map(trimval, 320, 580, -40, 40);
  uplidpulse = map(yval, 0, 1023, 400, 280);
  uplidpulse -= (trimval - 40);
  uplidpulse = constrain(uplidpulse, 280, 400);
  altuplidpulse = 680 - uplidpulse;

  lolidpulse = map(yval, 0, 1023, 410, 280);
  lolidpulse += (trimval / 2);
  lolidpulse = constrain(lolidpulse, 280, 400);
  altlolidpulse = 680 - lolidpulse;

  // closes eyelids
  pwm.setPWM(2, 0, 500);
  pwm.setPWM(3, 0, 240);
  pwm.setPWM(4, 0, 240);
  pwm.setPWM(5, 0, 500);

  delay(80);

  // opens eyelids to trimval value
  pwm.setPWM(2, 0, uplidpulse);
  pwm.setPWM(3, 0, lolidpulse);
  pwm.setPWM(4, 0, altuplidpulse);
  pwm.setPWM(5, 0, altlolidpulse);
}

void wakeup() {  //  Creature wakes up from sleep (blinks and looks around)

  Serial.println("WAKEUP");
  xval = 500;  // translate sensor information into servo info
  yval = 500;

  lexpulse = map(xval, 0, 1023, 220, 440);
  rexpulse = lexpulse;
  leypulse = map(yval, 0, 1023, 250, 500);
  reypulse = map(yval, 0, 1023, 400, 280);

  trimval = 650;  // this sets how wide the eyelids are positioned (higher number = wider eyes)
  trimval = map(trimval, 320, 580, -40, 40);
  uplidpulse = map(yval, 0, 1023, 400, 280);
  uplidpulse -= (trimval - 40);
  uplidpulse = constrain(uplidpulse, 280, 400);
  altuplidpulse = 680 - uplidpulse;

  lolidpulse = map(yval, 0, 1023, 410, 280);
  lolidpulse += (trimval / 2);
  lolidpulse = constrain(lolidpulse, 280, 400);
  altlolidpulse = 680 - lolidpulse;
  pwm.setPWM(0, 0, lexpulse);
  pwm.setPWM(1, 0, leypulse);

  pwm.setPWM(2, 0, uplidpulse);  //  opens eyelids
  pwm.setPWM(3, 0, lolidpulse);
  pwm.setPWM(4, 0, altuplidpulse);
  pwm.setPWM(5, 0, altlolidpulse);

  delay(100);

  blink();
  delay(500);
  blink();
  delay(500);

  pwm.setPWM(0, 0, 450);  // eyes glance right
  delay(800);
  pwm.setPWM(0, 0, 220);  // eyes glance left
  delay(1000);
  pwm.setPWM(0, 0, 330);  // eyes look forward
  delay(1000);

  blink();
  delay(200);
  blink();

  State = AWAKE;
}



void driftoff() {  //  Slowly closes creature's eyes + eyeballs roll up

  pwm.setPWM(0, 0, 330);  // centeres eyes on x-axis
  // blink();
  //blink();
  for (int i = 1; i <= 50; i++) {  // closes eyes slowly
    const double a = i / 50.0;
    pwm.setPWM(2, 0, uplidpulse + (400 - uplidpulse) * (a));
    pwm.setPWM(3, 0, lolidpulse + (240 - lolidpulse) * (a));
    pwm.setPWM(4, 0, altuplidpulse + (240 - altuplidpulse) * (a));
    pwm.setPWM(5, 0, altlolidpulse + (400 - altlolidpulse) * (a));
    pwm.setPWM(1, 0, 400 + (i));  // eyes roll up
    delay(40);
  }
  pwm.setPWM(2, 0, 460);  // closes eyelids completely
  pwm.setPWM(3, 0, 240);
  pwm.setPWM(4, 0, 240);
  pwm.setPWM(5, 0, 460);
  delay(1000);
  State = DORMANT;
}




void closeEyes() {
  Serial.println("closeEyes start");
  xval = 500;  // translate sensor information into servo info
  yval = 500;

  lexpulse = map(xval, 0, 1023, 220, 440);
  rexpulse = lexpulse;
  leypulse = map(yval, 0, 1023, 250, 500);
  reypulse = map(yval, 0, 1023, 400, 280);

  trimval = 650;  // this sets how wide the eyelids are positioned (higher number = wider eyes)
  trimval = map(trimval, 320, 580, -40, 40);
  uplidpulse = map(yval, 0, 1023, 400, 280);
  uplidpulse -= (trimval - 40);
  uplidpulse = constrain(uplidpulse, 280, 400);
  altuplidpulse = 680 - uplidpulse;

  lolidpulse = map(yval, 0, 1023, 410, 280);
  lolidpulse += (trimval / 2);
  lolidpulse = constrain(lolidpulse, 280, 400);
  altlolidpulse = 680 - lolidpulse;
  pwm.setPWM(0, 0, lexpulse);
  pwm.setPWM(1, 0, leypulse);
  pwm.setPWM(2, 0, 460);
  pwm.setPWM(3, 0, 240);
  pwm.setPWM(4, 0, 240);
  pwm.setPWM(5, 0, 460);


  delay(100);
  Serial.println("closeEyes finish");
}

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;

  pulselength = 1000000;  // 1,000,000 us per second
  pulselength /= 60;      // 60 Hz
  Serial.print(pulselength);
  Serial.println(" us per period");
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength);
  Serial.println(" us per bit");
  pulse *= 1000000;  // convert to us
  pulse /= pulselength;
  Serial.println(pulse);
}
