#include <Wire.h>
#include <XBee.h>
#include "CCSDS_Xbee/ccsds_xbee.h"

/* physical definitions */
#define TRIGGER_PIN 3
#define ARMED_LED_PIN 13
#define SCORCH_XBEE_ADDR 03
#define LINK_XBEE_ADDR 02
#define XBEE_PAN_ID 0x0B0B

/* function codes */
#define NOOP_FCNCODE 0x00
#define RESETCTR_FCNCODE 0x01
#define HK_REQ_FCNCODE 0x02
#define ARM_STATUS_FCNCODE 0x05
#define ARM_FCNCODE 0x0A
#define DISARM_FCNCODE 0x0D
#define FIRE_FCNCODE 0x0F

/* APIDs */
#define SCORCH_CMD_MSG_APID 200 // not used yet
#define SCORCH_HK_MSG_APID 210
#define SCORCH_STATUS_MSG_APID 220

/* behavioral constants */
#define CYCLE_DELAY 100 // time between execution cycles [ms]
#define ARM_TIMEOUT (60000/CYCLE_DELAY) // 60 * 1000 / CYCLE_DELAY

/* response definitions */
#define ARMED_RESPONSE 0xAA
#define INIT_RESPONSE 0xAC
#define READ_FAIL_RESPONSE 0xAF
#define BAD_COMMAND_RESPONSE 0xBB
#define DISARMED_RESPONSE 0xDD
#define FIRED_RESPONSE 0xFF

// Interface counters
uint16_t CmdExeCtr = 0;
uint16_t CmdRejCtr = 0;
uint32_t XbeeRcvdByteCtr = 0;
uint32_t XbeeSentByteCtr = 0;

/* function prototypes */
void fire();
void read_input();
void command_response(uint8_t *fcn_code);
void arm_system();
void disarm_system();
void one_byte_message(uint8_t msg);
void send_HK_pkt();

/*** program begin ***/

/* program variables */
boolean armed; // flag indicating if system is armed
int armed_ctr; // counter tracking number of cycles system has been armed

/* program functions */
void setup() {
	Serial.begin(9600);

	if(!InitXBee(SCORCH_XBEE_ADDR, XBEE_PAN_ID, Serial)) {
		// it initialized
		one_byte_message(INIT_RESPONSE);
	}
	else {
		// you're fucked
	}

	// disarm the system before we enable the pins
	delay(500);
	disarm_system(); // also sets armed to false and armed_ctr to -1
	
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(ARMED_LED_PIN, OUTPUT);
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
	int pkt_type;
	int bytes_read;
	uint8_t fcn_code;
	uint8_t incoming_bytes[100];

	if((pkt_type = readMsg(1)) == 0) {
		// Read something else, try again
	}
	// if we didn't have a read error, process it
	if (pkt_type > -1) {
    // FIXME: make this an actual byte counter
    XbeeRcvdByteCtr += 1;
		if (pkt_type) {
			bytes_read = readCmdMsg(incoming_bytes, fcn_code);
			command_response(&fcn_code);
		}
		else { // unknown packet type?
			one_byte_message(READ_FAIL_RESPONSE);
      CmdRejCtr++;
		}
	}
}

void command_response(uint8_t *fcncode) {
  // process a noop command
  if(*fcncode == NOOP_FCNCODE){
    CmdExeCtr++;
  }
  // process a command to send an HK pkt
	else if(*fcncode == HK_REQ_FCNCODE){
    send_HK_pkt();
    CmdExeCtr++;
  }
  // process a command to reset the counters
  else if(*fcncode == RESETCTR_FCNCODE){
    CmdExeCtr = 0;
    CmdRejCtr = 0;
    XbeeRcvdByteCtr = 0;
    XbeeSentByteCtr = 0;
    CmdExeCtr++;
  }
  // process a command to arm the system
	else if(*fcncode == ARM_FCNCODE){
		arm_system();
    CmdExeCtr++;
	}
	// process a command to disarm the system
	else if(*fcncode == DISARM_FCNCODE){
		disarm_system();
    CmdExeCtr++;
	}
	// process a command to FIIIIRRRRREEEEEE!
	else if(*fcncode == FIRE_FCNCODE){
		// fire 
		fire();
    CmdExeCtr++;
	}
	// process a command to report the arm status
	else if(*fcncode == ARM_STATUS_FCNCODE){
		if(armed) {
			one_byte_message(ARMED_RESPONSE);
		}
		else {
			one_byte_message(DISARMED_RESPONSE);
		}
    CmdExeCtr++;
	}
	else {
		one_byte_message(BAD_COMMAND_RESPONSE);
    CmdRejCtr++;
	}
}

void arm_system(){
	armed = true;
	digitalWrite(ARMED_LED_PIN, HIGH);
	armed_ctr = 1;

	one_byte_message(ARMED_RESPONSE);
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
	delay(500);
	disarm_system();
}

void one_byte_message(uint8_t msg) {
	uint8_t tlm_data;
	addIntToTlm<uint8_t>(msg, &tlm_data, (uint16_t)0);
	sendTlmMsg(LINK_XBEE_ADDR, SCORCH_STATUS_MSG_APID, &tlm_data, (uint16_t)1);
  XbeeSentByteCtr += 13;
}

void send_HK_pkt(){

  uint8_t tlm_data[28];
  uint16_t payloadSize = 0;
  
  payloadSize = addIntToTlm(CmdExeCtr, tlm_data, payloadSize); // Add counter of sent packets to message
  payloadSize = addIntToTlm(CmdRejCtr, tlm_data, payloadSize); // Add counter of sent packets to message
  payloadSize = addIntToTlm(XbeeRcvdByteCtr, tlm_data, payloadSize); // Add counter of sent packets to message
  payloadSize = addIntToTlm(XbeeSentByteCtr, tlm_data, payloadSize); // Add counter of sent packets to message
  payloadSize = addIntToTlm(millis()/1000L, tlm_data, payloadSize); // Timer

  sendTlmMsg(LINK_XBEE_ADDR, SCORCH_HK_MSG_APID, tlm_data, sizeof(tlm_data));
  XbeeSentByteCtr += sizeof(tlm_data);
}
