#include<Wire.h>
#include<XBee.h>
#include"ccsds_xbee.h"

// physical definitions
#define TRIGGER_PIN 3
#define ARMED_LED_PIN 13
#define XBEE_ADDR 03
#define TLM_ADDR 04
#define XBEE_PAN_ID 0x0B0B
#define ARM_FCNCODE 0x0A
#define ARM_STATUS_FCNCODE 0x01
#define DISARM_FCNCODE 0x0D
#define FIRE_FCNCODE 0x0F

// function prototypes
void fire();
void read_input();
void command_response(uint8_t _fcn_code, uint8_t data[], uint8_t length);
void arm_system();
void disarm_system();

/* program begin */

// program variables
boolean armed;
int pkt_type;
int bytes_read;
uint8_t incoming_bytes[100];
uint8_t fcn_code;
uint8_t tlm_pos = 0;
uint8_t tlm_data[1];

void setup() {
	
	// disarm the system before we enable the pins
	disarm_system();
	
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(ARMED_LED_PIN, OUTPUT);

	Serial.begin(9600);

	if(!InitXBee(XBEE_ADDR, XBEE_PAN_ID, Serial)) {
		// it initialized
		tlm_pos = 0;
		tlm_pos = addIntToTlm<uint8_t>(0xAC, tlm_data, tlm_pos);
		sendTlmMsg(TLM_ADDR, tlm_data, tlm_pos);
	}
	else {
		// you're fucked
	}

	armed = false;
	pkt_type = 0;
	bytes_read = 0;
	fcn_code = 0;
}

uint8_t counter = 0;

void loop() {
	// look for any new messages
	read_input();
	// wait
	delay(100);
}

void read_input() {
	if((pkt_type = readMsg(1)) == 0) {
		// Read something else, try again
	}
	// if we didn't have a read error, process it
	if (pkt_type > -1) {
		if (pkt_type) {
			bytes_read = readCmdMsg(incoming_bytes, fcn_code);
			command_response(fcn_code, incoming_bytes, bytes_read);
	}
		else {
			tlm_pos = 0;
			tlm_pos = addIntToTlm<uint8_t>(0xAF, tlm_data, tlm_pos);
			sendTlmMsg( TLM_ADDR, tlm_data, tlm_pos);
		}
	}
}

void command_response(uint8_t _fcncode, uint8_t data[], uint8_t length) {
	// process a command to arm the system
	if(_fcncode == ARM_FCNCODE){
		arm_system();
	}
	// process a command to disarm the system
	else if(_fcncode == DISARM_FCNCODE){
		disarm_system();
	}
	// process a command to FIIIIRRRRREEEEEE!
	else if(_fcncode == FIRE_FCNCODE){
		// fire 
		fire();
		// disarm the system again to prevent repeated firings
		disarm_system();
	}
	// process a command to report the arm status
	else if(_fcncode == ARM_STATUS_FCNCODE){

		tlm_pos = 0;
		// telemetry compilation
		tlm_pos = addIntToTlm(armed, tlm_data, tlm_pos);
		// send the message
		sendTlmMsg( TLM_ADDR, tlm_data, tlm_pos);
	}
	else {
		tlm_pos = 0;
		tlm_pos = addIntToTlm<uint8_t>(0xBB, tlm_data, tlm_pos);
		sendTlmMsg( TLM_ADDR, tlm_data, tlm_pos);
	}
	
}

void arm_system(){
	armed = true;
	digitalWrite(ARMED_LED_PIN, HIGH);
	tlm_pos=0;
	tlm_pos = addIntToTlm<uint8_t>(0xAA, tlm_data, tlm_pos);
	sendTlmMsg( TLM_ADDR, tlm_data, tlm_pos);
}

void disarm_system(){
	armed = false;
	digitalWrite(ARMED_LED_PIN, LOW);
}

void fire() {
	// if the system is armed, fire
	if(armed){
		digitalWrite(TRIGGER_PIN, HIGH);
		delay(3000);
		digitalWrite(TRIGGER_PIN, LOW);
		tlm_pos=0;
		tlm_pos = addIntToTlm<uint8_t>(0xFF, tlm_data, tlm_pos);
		sendTlmMsg( TLM_ADDR, tlm_data, tlm_pos);}
}
