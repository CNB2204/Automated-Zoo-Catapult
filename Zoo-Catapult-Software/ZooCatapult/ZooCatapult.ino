#include <Servo.h>

// --- PIN DEFINITIONS ---
const int GO_BTN      = 5;   
const int STOP_BTN    = A0;  
const int RESET_BTN   = A1;  

const int ROTATOR_PIN = 4;   
const int AUGER_STEP  = 7;   
const int AUGER_DIR   = 6;   
const int ACT_IN1     = 8;   
const int ACT_IN2     = 9;   
const int LAZY_SUSAN  = 12;  

const int STEP_DELAY_US = 900; 

Servo armRotator;
Servo lazySusan;

bool systemLocked = false; 

void setup() {
  Serial1.begin(9600);   
  Serial1.begin(9600);  
  
  // Use INPUT_PULLUP to keep pins "High" so they don't float and trigger on their own
  pinMode(GO_BTN, INPUT_PULLUP);
  pinMode(STOP_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);
  
  pinMode(AUGER_STEP, OUTPUT);
  pinMode(AUGER_DIR, OUTPUT);
  pinMode(ACT_IN1, OUTPUT);
  pinMode(ACT_IN2, OUTPUT);

  armRotator.attach(ROTATOR_PIN);
  lazySusan.attach(LAZY_SUSAN);

  stopEverything();
  
  // Give the ESP32 a second to boot so its startup "noise" doesn't trigger a launch
  delay(2000); 
  while(Serial1.available() > 0) Serial1.read(); // Flush the buffer
  
  Serial.println(">>> SYSTEM STABILIZED: WAITING FOR USER INPUT");
}

void loop() {
  // 1. Check Wireless (Only process if it's EXACTLY 'G', 'S', or 'R')
  if (Serial1.available() > 0) {
    char cmd = Serial1.read();
    if (cmd == 'G' || cmd == 'S' || cmd == 'R') {
       processCommand(cmd, "WIRELESS");
    }
  }

  // 2. Check Buttons with basic debounce (Ensures a real press)
  if (digitalRead(STOP_BTN) == LOW) {
    processCommand('S', "PHYSICAL");
  }
  
  if (digitalRead(RESET_BTN) == LOW) {
    delay(50); // Debounce
    if(digitalRead(RESET_BTN) == LOW) processCommand('R', "PHYSICAL");
  }

  if (digitalRead(GO_BTN) == LOW && !systemLocked) {
    delay(100); // Strict debounce
    if(digitalRead(GO_BTN) == LOW) processCommand('G', "PHYSICAL");
  }
}

void processCommand(char command, String source) {
  if (command == 'S') {
    stopEverything();
    systemLocked = true;
    Serial.println("[" + source + "] !!! STOP COMMAND RECEIVED !!!");
  }
  else if (command == 'R') {
    systemLocked = false;
    Serial.println("[" + source + "] System Reset/Unlocked.");
    handleHoming();
  }
  else if (command == 'G' && !systemLocked) {
    Serial.println("[" + source + "] User Initiated Launch...");
    runFullSequence();
  }
}

// --- SEQUENTIAL LOGIC ---
void runFullSequence() {
  // Step 1: Tension
  armRotator.writeMicroseconds(1700); 
  if (!waitWithSafety(6000)) return; 
  armRotator.writeMicroseconds(1500); 

  // Step 2: Lock
  digitalWrite(ACT_IN1, HIGH);
  digitalWrite(ACT_IN2, LOW);
  if (!waitWithSafety(7000)) return; 
  stopActuator();

  // Step 3: Feed
  if (!runAuger(5000)) return; 

  // Step 4: Slacken
  armRotator.writeMicroseconds(1300); 
  if (!waitWithSafety(5000)) return; 
  armRotator.writeMicroseconds(1500);

  // Step 5: Aim
  lazySusan.write(90); 
  if (!waitWithSafety(2000)) return;

  // Step 6: FIRE
  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, HIGH);
  if (!waitWithSafety(3000)) return; 
  
  stopEverything();
}

// --- UTILITIES ---

bool waitWithSafety(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (digitalRead(STOP_BTN) == LOW) { stopEverything(); systemLocked = true; return false; }
    if (Serial1.available() > 0) {
      if (Serial1.read() == 'S') { stopEverything(); systemLocked = true; return false; }
    }
  }
  return true; 
}

bool runAuger(unsigned long ms) {
  digitalWrite(AUGER_DIR, HIGH);
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (digitalRead(STOP_BTN) == LOW) { stopEverything(); systemLocked = true; return false; }
    digitalWrite(AUGER_STEP, HIGH);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(AUGER_STEP, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }
  return true;
}

void handleHoming() {
  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, HIGH); 
  delay(3000);
  stopActuator();
}

void stopEverything() {
  armRotator.writeMicroseconds(1500);
  lazySusan.writeMicroseconds(1500);
  stopActuator();
  digitalWrite(AUGER_STEP, LOW);
}

void stopActuator() {
  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, LOW);
}