
class Sensor {

  public:
    int trigPin ;
    int echoPin ;
    unsigned long duration, distance;


    //----Constructor
    Sensor(int trig, int echo) {
      trigPin = trig;
      echoPin = echo;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);

    }

    //----Calculate Distance
    int dist() {

      digitalWrite(trigPin, LOW); //Set trigger pin low
      delayMicroseconds(10); //Let signal settle
      digitalWrite(trigPin, HIGH); //Set trigPin high
      delayMicroseconds(15); //Delay in high state
      digitalWrite(trigPin, LOW); //ping has now been sent


      //----CALCULATION----
      duration = pulseIn(echoPin, HIGH, 10000);
      distance = duration / 58;

      return distance;
    }

};
//=====END CLASS======

