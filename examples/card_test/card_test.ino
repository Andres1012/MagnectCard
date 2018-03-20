#include <MagnectCard.h>

MagnectCard reader;

void setup() {
  Serial.begin(115200);
  reader.init();
  // put your setup code here, to run once:
}

void loop() {
  delay(50);
  if (reader.available()) {
    Serial.println(reader.getLectorID());
  }
  // put your main code here, to run repeatedly:

}
