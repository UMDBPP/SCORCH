# SCORCH
SCORCH Payload for UMDBPP

CCSDS.cpp and .h define the CCSDS packet structure and should not need to be modified by the user.
ccsds_xbee.cpp and .h provide an "easy-to-use" interface which takes care of formatting outbound commands and telemetry into CCSDS packets, decoding incomming command and telemetry packets, and interfacing with the Xbee's packet mode. It should not need to be modified by the user.

SCORCH.ino is the main file and contains the setup function, loop function, and all the SCORCH-specific functionality.

SCORCH_XbeeCmds.xml can be imported into XCTU and contains the definitions of the Arm, Disarm, ArmStatus, and Fire commands so that a user does not have to define the packet structure themselves.