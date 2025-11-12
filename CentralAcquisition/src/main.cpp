#include <Arduino.h>
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"
#include <string.h>
#include <Wire.h> // Include I2C library for Sprint 2/3

//  |Protocol Messages|
const char * CONNECT_MSG = "CONNECT";
const char * DISCONNECT_MSG = "DISCONNECT";
const char * EXAMINATION_MSG = "EXAM";
const char * DOSE_MSG = "DOSE";

// |Hardware Definitions|
#define LED_EXAM_SET_PIN 8  // Red LED: an exam is selected
#define LED_NO_EXAM_PIN  9  // Green LED: no exam selected (NONE)
// TODO: Add pin definitions for Prepare Button and Xray Button

// |Global Variables|
static EXAMINATION_TYPES currentExamType = EXAM_TYPE_NONE;

// |Event Definitions|
typedef enum {
	EV_CONNECT_MSG_RECEIVED, 
	EV_DISCONNECT_MSG_RECEIVED, 
	EV_EXAM_MSG_RECEIVED,
    // TODO: Add events for buttons
    // EV_PREPARE_BUTTON_PRESSED,
    // EV_PREPARE_BUTTON_RELEASED,
    // EV_XRAY_BUTTON_PRESSED,
    // EV_XRAY_BUTTON_RELEASED,
	EV_NONE
} EVENTS;

// |State Machine Definitions|
// Main states
typedef enum {
    STATE_DISCONNECTED,
    STATE_CONNECTED
} CENTRAL_ACQ_MAIN_STATE;

// Substates for STATE_CONNECTED
typedef enum {
    SUBSTATE_IDLE,
    SUBSTATE_PREPARING,
    SUBSTATE_PREPARED,
    SUBSTATE_ACQUIRING
} CENTRAL_ACQ_SUBSTATE;

// Static variables to hold the current state
static CENTRAL_ACQ_MAIN_STATE mainState = STATE_DISCONNECTED;
static CENTRAL_ACQ_SUBSTATE subState = SUBSTATE_IDLE;


// |Function Prototypes|
EVENTS getEvent(void);
void handleEvent(EVENTS event);
static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]);
void updateLeds(void);


// |Arduino Setup & Loop|
void setup() {
  Serial.begin(9600);
  Wire.begin(); // Initialize I2C as master [cite: 58-73]
  
  pinMode(LED_EXAM_SET_PIN, OUTPUT);
  pinMode(LED_NO_EXAM_PIN, OUTPUT);
  
  updateLeds(); // Update LEDs to initial state (green on)
  // TODO: Initialize button pins (INPUT_PULLUP)
}

void loop() {
    handleEvent(getEvent());

    // The dummy dose code remains for now
    static unsigned long timeOut = millis();
    static int doseCnt = 0;
    unsigned long curTime = millis();
    if (mainState == STATE_CONNECTED && curTime > timeOut) { // Only send when connected
        Serial.print("$dose:"); Serial.print(doseCnt); Serial.println("#");
        timeOut = curTime + 5000;
        doseCnt++;
    }
}


// |State Machine Implementation|

void handleEvent(EVENTS event)
{
    // Handle transitions that can happen from *any* state
    if (event == EV_DISCONNECT_MSG_RECEIVED) {
        mainState = STATE_DISCONNECTED;
        subState = SUBSTATE_IDLE;
        currentExamType = EXAM_TYPE_NONE;
        updateLeds();
        writeMsgToSerialPort(DISCONNECT_MSG);
        // TODO: Switch off all lamps and set SAN_XRAY_ENABLED low
        return; // Event handled
    }

    switch (mainState) {
        
        case STATE_DISCONNECTED:
            // In this state, we *only* care about the CONNECT message
            if (event == EV_CONNECT_MSG_RECEIVED) {
                mainState = STATE_CONNECTED;
                subState = SUBSTATE_IDLE; // Enter the default substate
                writeMsgToSerialPort(CONNECT_MSG);
            }
            break;

        case STATE_CONNECTED:
            // Handle events based on the *substate*
            switch (subState) {
                
                case SUBSTATE_IDLE:
                    if (event == EV_EXAM_MSG_RECEIVED) {
                        updateLeds();
                        // TODO: Send ACK to PC
                        // TODO: Send exam type to Geometry & XrayGen via I2C
                    }
                    // else if (event == EV_PREPARE_BUTTON_PRESSED && currentExamType != EXAM_TYPE_NONE) {
                    //     subState = SUBSTATE_PREPARING;
                    //     // TODO: Send "Prepare" command via I2C
                    // }
                    break;

                case SUBSTATE_PREPARING:
                    // TODO: Implement logic for this state
                    // if (event == EV_PREPARE_BUTTON_RELEASED) {
                    //     subState = SUBSTATE_IDLE;
                    // }
                    // if (geometry_ready && xray_ready) {
                    //     subState = SUBSTATE_PREPARED;
                    // }
                    // if (timeout) {
                    //     subState = SUBSTATE_IDLE;
                    // }
                    break;

                case SUBSTATE_PREPARED:
                    // TODO: Implement logic for this state
                    // if (event == EV_XRAY_BUTTON_PRESSED) {
                    //     subState = SUBSTATE_ACQUIRING;
                    // }
                    // if (event == EV_PREPARE_BUTTON_RELEASED) { // Not in PDF, but logical
                    //     subState = SUBSTATE_IDLE;
                    // }
                    break;

                case SUBSTATE_ACQUIRING:
                    // TODO: Implement logic for this state
                    // if (event == EV_XRAY_BUTTON_RELEASED) {
                    //     subState = SUBSTATE_IDLE;
                    //     currentExamType = EXAM_TYPE_NONE;
                    //     updateLeds();
                    //     // TODO: Get dose from XrayGen (I2C) and send to PC
                    // }
                    break;
            }
            break; // End case STATE_CONNECTED
    }
}


// |Event & Message Handling|

EVENTS getEvent() 
{
    char msg[MAX_MSG_SIZE];
    
    // 1. Check for Serial messages
    if (checkForMsgOnSerialPort(msg)) {
        if      (strcmp(msg, CONNECT_MSG) == 0)     return EV_CONNECT_MSG_RECEIVED;
        else if (strcmp(msg, DISCONNECT_MSG) == 0)  return EV_DISCONNECT_MSG_RECEIVED;
        
        else if (strncmp(msg, EXAMINATION_MSG, strlen(EXAMINATION_MSG)) == 0 &&
                 msg[strlen(EXAMINATION_MSG)] == MSG_ARGUMENT_SEPARATOR) {
            
            int examTypeInt = atoi(msg + strlen(EXAMINATION_MSG) + 1);
            if (examTypeInt >= EXAM_TYPE_SINGLE_SHOT && examTypeInt <= EXAM_TYPE_NONE) {
                currentExamType = (EXAMINATION_TYPES)examTypeInt;
            } else {
                currentExamType = EXAM_TYPE_NONE;
            }
            return EV_EXAM_MSG_RECEIVED;
        }
    }

    // 2. TODO: Check for button presses
    // ... logic to read buttons and return events like EV_PREPARE_BUTTON_PRESSED ...

    return EV_NONE;
}

static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE])
{
	Serial.write(MSG_START_SYMBOL);
	int i = 0;
	while (i < MAX_MSG_SIZE && msg[i] != '\0') {
		Serial.write(msg[i++]);
	}
	Serial.write(MSG_END_SYMBOL);
	return true;
}

// |Message Receiving State|
// This enum defines the simple state machine for reading a message byte-by-byte
typedef enum {
	WAITING_FOR_MSG_START_SYMBOL, 
	WAITING_FOR_MSG_END_SYMBOL
} MSG_RECEIVE_STATE;

bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE])
{
    static MSG_RECEIVE_STATE msgRcvState = WAITING_FOR_MSG_START_SYMBOL;
    static int receiveIndex = 0;
    static char msg[MAX_MSG_SIZE] {0};

    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
		switch (msgRcvState) {
			case WAITING_FOR_MSG_START_SYMBOL:
				if (receivedChar == MSG_START_SYMBOL) {
					receiveIndex = 0;
					msgRcvState = WAITING_FOR_MSG_END_SYMBOL;
				}
				break;
			case WAITING_FOR_MSG_END_SYMBOL:
				if (receivedChar == MSG_END_SYMBOL) {
					msg[receiveIndex] = '\0';
                    receiveIndex = 0;
                    strncpy(msgArg, msg, MAX_MSG_SIZE);  
					msgRcvState = WAITING_FOR_MSG_START_SYMBOL;
					return true;
				}
				else msg[receiveIndex++] = receivedChar;
				break;
			default:
				break;
		}
	}
    return false;
}

void updateLeds(void)
{
    if (currentExamType == EXAM_TYPE_NONE) {
        digitalWrite(LED_NO_EXAM_PIN, HIGH);  // Green on
        digitalWrite(LED_EXAM_SET_PIN, LOW); // Red off
    } else {
        digitalWrite(LED_NO_EXAM_PIN, LOW);   // Green off
        digitalWrite(LED_EXAM_SET_PIN, HIGH); // Red on
    }
}
