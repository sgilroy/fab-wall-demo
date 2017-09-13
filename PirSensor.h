class PirSensor {
  public:

    enum motionTransitions {
        NO_CHANGE,
        START,
        END
    };
    
    int inputPin = 3;               // choose the input pin (for PIR sensor)
    int pirState = LOW;             // we start, assuming no motion detected
    int val = 0;                    // variable for reading the pin status

    PirSensor(int _inputPin) {
      inputPin = _inputPin;
      pinMode(inputPin, INPUT);     // declare sensor as input
    }

    enum motionTransitions update() {
      val = digitalRead(inputPin);  // read input value
      if (val == HIGH) {            // check if the input is HIGH
        if (pirState == LOW) {
          // we have just turned on
          Serial.println("Motion detected!");
          // We only want to print on the output change, not state
          pirState = HIGH;
          return START;
        }
      } else {
        if (pirState == HIGH) {
          // we have just turned of
          Serial.println("Motion ended!");
          // We only want to print on the output change, not state
          pirState = LOW;
          return END;
        }
      }
      return NO_CHANGE;
    }
};

