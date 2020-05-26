

bool METRIC = false;

// nema 14 w 19:1 = 3800 steps/rev
// nema 11 w 14:1 = 2800 steps/rev
int STEPS_PER_REV = 2800 * 4;
int DISTANCE_PER_REV = 8;

int ENABLE = 2;
int DIRECTION = 3;
int PULSE = 4;


const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

int speed = 0;
float distance = 0.0;

boolean newData = false;


void setup() {

  pinMode(ENABLE, OUTPUT);
  pinMode(DIRECTION, OUTPUT);
  pinMode(PULSE, OUTPUT);

  digitalWrite(ENABLE, LOW);

  digitalWrite(PULSE, LOW);

  Serial.begin(115200);

  while (!Serial) {}

}

void loop() {


  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    showParsedData();

    setDirection();

    move(stepsCalculation(distance), timingCalculation(speed));

    newData = false;
  }

}

int timingCalculation(int speed) {

  // Input should be speed in mm/s

  // Steps per millimeter
  int stepsPerMillimeter = STEPS_PER_REV / DISTANCE_PER_REV;

  // Calculate number of microseconds between steps to travel one mm/s
  int timing = 1000000 / stepsPerMillimeter;

  return (timing / speed);

}

long stepsCalculation(float distance) {

  // distance must be in mm

  float revolutions = abs(distance) / DISTANCE_PER_REV;

  return long(STEPS_PER_REV * revolutions);

}

void setDirection() {

  if (distance > 0) {

    digitalWrite(DIRECTION, LOW);

  } else {

    digitalWrite(DIRECTION, HIGH);
    
  }

}

void move(long steps, int timing) {

  for (long i = 0; i <= steps; i++) {

    digitalWrite(PULSE, LOW);

    delayMicroseconds(timing / 2);

    digitalWrite(PULSE, HIGH);

    delayMicroseconds(timing / 2);

  }

}


void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, " "); // Distance
  distance = atof(strtokIndx);         // Convert to float

  strtokIndx = strtok(NULL, " "); // this continues where the previous call left off
  speed = atoi(strtokIndx);       // convert to an integer

}

void showParsedData() {

  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print("Speed: ");
  Serial.println(speed);

  Serial.print("Steps: ");
  Serial.println(stepsCalculation(distance));
  Serial.print("Timing: ");
  Serial.println(timingCalculation(speed));
}
