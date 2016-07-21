#include<Wire.h>
#include<XBee.h>
#include"CCSDS_Xbee/ccsds_xbee.h"

/* physical definitions */
#define TRIGGER_PIN 3
#define ARMED_LED_PIN 13
#define XBEE_ADDR 03
#define TLM_ADDR 02
#define XBEE_PAN_ID 0x0B0B

/* function codes */
#define ARM_FCNCODE 0x0A
#define ARM_STATUS_FCNCODE 0x01
#define DISARM_FCNCODE 0x0D
#define FIRE_FCNCODE 0x0F

/* behavioral constants */
#define CYCLE_DELAY 100 // time between execution cycles [ms]
#define ARM_TIMEOUT (60000/CYCLE_DELAY) // 60 * 1000 / CYCLE_DELAY

/* response definitions */
#define INIT_RESPONSE 0xAC
#define READ_FAIL_RESPONSE 0xAF
#define BAD_COMMAND_RESPONSE 0xBB
#define ARMED_RESPONSE 0xAA
#define DISARMED_RESPONSE 0xDD
#define FIRED_RESPONSE 0xFF

/* function prototypes */
void fire();
void read_input();
void command_response(uint8_t _fcn_code, uint8_t data[], uint8_t length);
void arm_system();
void disarm_system();
void one_byte_message(uint8_t msg);

/*** program begin ***/

/* program variables */
boolean armed;
int pkt_type;
int bytes_read;
uint8_t incoming_bytes[100];
uint8_t fcn_code;
uint8_t tlm_pos;
uint8_t tlm_data[1];
int armed_ctr; // counter tracking number of cycles system has been armed

/* program functions */
void setup() {
	// disarm the system before we enable the pins
	disarm_system();

	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(ARMED_LED_PIN, OUTPUT);

	Serial.begin(9600);

	if(!InitXBee(XBEE_ADDR, XBEE_PAN_ID, Serial)) {
		// it initialized
		one_byte_message(INIT_RESPONSE);
	}
	else {
		// you're fucked
	}

	armed = false;
	pkt_type = 0;
	bytes_read = 0;
	fcn_code = 0;
	tlm_pos = 0;
	armed_ctr = -1;
}

void loop() {
	// look for any new messages
	read_input();
	// if system is armed, increment the timer indicating for how long
	if(armed_ctr > 0){
		armed_ctr++;
	}
	// if the system has been armed for more than the timeout, disarm
	if(armed_ctr > ARM_TIMEOUT){
		disarm_system();
	}
	// wait
	delay(CYCLE_DELAY);
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
		else { // unknown packet type?
			one_byte_message(READ_FAIL_RESPONSE);
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
	}
	// process a command to report the arm status
	else if(_fcncode == ARM_STATUS_FCNCODE){
		if(armed) {
			one_byte_message(ARMED_RESPONSE);
		} else {
			one_byte_message(DISARMED_RESPONSE);
		}
	}
	else {
		one_byte_message(BAD_COMMAND_RESPONSE);
	}
}

void arm_system(){
	armed = true;
	digitalWrite(ARMED_LED_PIN, HIGH);

	one_byte_message(ARMED_RESPONSE);

	armed_ctr = 1;
}

void disarm_system(){
	armed = false;
	digitalWrite(ARMED_LED_PIN, LOW);
	armed_ctr = -1;
  one_byte_message(DISARMED_RESPONSE);

}

void fire() {
	// if the system is armed, fire
	if(armed){
		digitalWrite(TRIGGER_PIN, HIGH);
		delay(3000);
		digitalWrite(TRIGGER_PIN, LOW);
		one_byte_message(FIRED_RESPONSE);
	}
	// disarm the system again to prevent repeated firing attempts
	disarm_system();
}

void one_byte_message(uint8_t msg) {
	tlm_pos = 0;
	tlm_pos = addIntToTlm<uint8_t>((uint8_t)msg, tlm_data, tlm_pos);
	sendTlmMsg(TLM_ADDR, tlm_data, tlm_pos);
}
