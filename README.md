# SCORCH
SCORCH Payload for UMDBPP

String Cutdown Operated by Rocket Combustion Heat

## About
CCSDS.cpp and .h define the CCSDS packet structure and should not need to be modified by the user.

ccsds_xbee.cpp and .h provide an "easy-to-use" interface which takes care of formatting outbound commands and telemetry into CCSDS packets, decoding incomming command and telemetry packets, and interfacing with the Xbee's packet mode. It should not need to be modified by the user.

SCORCH.ino is the main file and contains the setup function, loop function, and all the SCORCH-specific functionality.

SCORCH_XbeeCmds.xml can be imported into XCTU and contains the definitions of the Arm, Disarm, ArmStatus, and Fire commands so that a user does not have to define the packet structure themselves.

## Usage

1) Flip power switch, observe power up sequence (red LED will blink in 3 sets of 3 quick blinks)

2) Wait 10seconds for xbee to initalize

3) Flip arm switch on box to 'ARMED'

4) Send Arm Cmd

5) Recieve packet with payload 0xAA to confirm arm

6) Send Fire Cmd

7) Receive packet with payload 0xFF which indicates firing is happening

8) Send ArmStatus Cmd to verify that payload is armed

9) Receive packet with payload 0x00 to confirm disarm after firing

10) Flip arm switch on box to 'DISARM'

11) Flip power switch to off
