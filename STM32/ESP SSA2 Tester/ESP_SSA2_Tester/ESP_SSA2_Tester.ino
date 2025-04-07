const int bufferSize = 100;
int buffer[bufferSize];
int bufferIndex = 0;

const int dacPin = 25;
int dacValue = 0;

void setup() {
  analogReadResolution(12);
  analogSetWidth(12);
  pinMode(13, INPUT);
  Serial.begin(9600);
  delay(1000);

  dacWrite(dacPin, dacValue);
}

void loop() {
  int val = analogRead(13);
  buffer[bufferIndex] = val;
  bufferIndex = (bufferIndex + 1) % bufferSize;

  long sum = 0;
  for (int i = 0; i < bufferSize; i++) {
    sum += buffer[i];
  }
  int avg = sum / bufferSize;

  Serial.print(avg);
  Serial.print('\t');
  Serial.println(dacValue);

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    int mA = input.toInt();  // input in milliamps
    if (mA >= 0 && mA <= 4000) {
      dacValue = (int)(mA * 255.0 / 4000.0);
      dacWrite(dacPin, dacValue);
    } else {
      Serial.println("Invalid input! Enter mA from 0 to 4000.");
    }
  }

  delay(20);
}
