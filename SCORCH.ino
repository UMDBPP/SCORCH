#include<Wire.h>
#include<XBee.h>
#include"ccsds_xbee.h"

// physical definitions
#define TRIGGER_PIN 3
#define LED_PIN 13
#define XBEE_ADDR 03
#define XBEE_PAN_ID 0x0B0B

// function prototypes
void fire();
void read_input();
void command_response(uint8_t _fcn_code, uint8_t data[], uint8_t length);

/* program begin */

// program variables
boolean fired;
int pkt_type;
int bytes_read;
uint8_t incoming_bytes[100];
uint8_t fcn_code;

void setup() {
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(TRIGGER_PIN, LOW);
	Serial.begin(9600);

	if(!InitXBee(XBEE_ADDR, XBEE_PAN_ID, Serial)) {
		// it initialized
	}
	else {
		// you're fucked
	}

	fired = false;
	pkt_type = 0;
	bytes_read = 0;
	fcn_code = 0;
}

void loop() {
	if (!fired) {
		read_input();
	}
	else {
		digitalWrite(LED_PIN, HIGH);
		delay(1000);
		digitalWrite(LED_PIN, LOW);
		delay(1000);
	}
}

void read_input() {
	if((pkt_type = readMsg(1)) == 0) {
		// Read something else, try again
		pkt_type = readMsg(1);
	}
	if (pkt_type > -1) {
		if (pkt_type) {
			bytes_read = readCmdMsg(incoming_bytes, fcn_code);
			command_response(fcn_code, incoming_bytes, bytes_read);
		}
	}
}

void command_response(uint8_t _fcn_code, uint8_t data[], uint8_t length) {
	fire();
}

void fire() {
	digitalWrite(TRIGGER_PIN, HIGH);
	delay(3000);
	digitalWrite(TRIGGER_PIN, LOW);
}
