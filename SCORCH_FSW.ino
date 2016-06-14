const int triggerPin = 3;
const int ledPin = 13;

void setup {
	pinMode(triggerPin, OUTPUT);
	pinMode(ledPin, OUTPUT);
	digitalWrite(triggerPin, LOW);
	digitalWrite(ledPin, LOW);
	delay(10000);
	digitalWrite(triggerPin, HIGH);
	delay(3000);
	digitalWrite(triggerPin, LOW);
}

void loop {
	digitalWrite(ledPin, HIGH);
	delay(1000);
	digitalWrite(ledPin, LOW);
	delay(1000);
}
