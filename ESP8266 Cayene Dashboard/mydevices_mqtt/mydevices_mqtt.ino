#include <CayenneMQTTESP8266.h>


char ssid[] = "Gator";
char password[] = "gators123";

char username[] = "02ed3e70-660a-11ea-b301-fd142d6c1e6c";
char mqtt_password[] = "8ce568232d8b56ecd2a915b86130385f27e744b8";
char client_id[] = "9c95d540-660b-11ea-ba7c-716e7f5ba423";
int8_t MQ_135, MQ_9, MQ_2, Temperature, Distance;
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 3000;           // interval at which to blink (milliseconds)

void setup() {
  // put your setup code here, to run once:
  Cayenne.begin(username, mqtt_password, client_id, ssid, password);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Cayenne.loop();
  if (Serial.available() > 0) {
    MQ_135 = Serial.read();
    MQ_9 = Serial.read();
    MQ_2 = Serial.read();
    Temperature = Serial.read();
    }
     unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    Cayenne.virtualWrite(1, MQ_135, "Random", "%");
    Cayenne.virtualWrite(2, MQ_9, "Random", "%");
    Cayenne.virtualWrite(3, MQ_2, "Random", "%");
    Cayenne.virtualWrite(4, Temperature, "Random", "%");
    }
}

CAYENNE_IN(0){
  digitalWrite(2, !getValue.asInt());
  }
