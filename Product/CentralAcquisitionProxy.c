#include "CentralAcquisitionProxy.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // sleep
#include "serialPort.h"

static bool setupSerialConnection(void);
static bool connect(void);
static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
static bool getMsgFromCentralAcquisition(char msg[MAX_MSG_SIZE]);


const char * CONNECT_MSG = "CONNECT";
const char * DISCONNECT_MSG = "DISCONNECT";
const char * EXAMINATION_MSG = "EXAM";  // Remark that this msg will have an argument.
                                        // For instance the actual msg could be EXAM:0 
										// The 0 indicates a single shot exam
const char * DOSE_MSG = "DOSE";			// Will also have an argument


bool connectWithCentralAcquisition(void)
{
	bool trySucceeded = false;
	if (setupSerialConnection()) {
		trySucceeded = connect(); 
	}
	return trySucceeded;
}

bool disconnectFromCentralAcquisition(void)
{
	char receivedMsg[MAX_MSG_SIZE];
	int tryCount = 0;
	const int maxTryCount = 10;
	bool disConnected = false;
	
	printf("Try to disconnect from CentralAquisition...  ");
	do {
		writeMsgToSerialPort(DISCONNECT_MSG);
		if (getMsgFromCentralAcquisition(receivedMsg)) {
			if (strcmp(receivedMsg, DISCONNECT_MSG) == 0) disConnected = true;
			else 									      tryCount++;			
		}
		else tryCount++;
		sleep(1);
	} while (!disConnected && tryCount < maxTryCount);
	
	if (disConnected) closeSerialPort();
		
	if (disConnected) printf("  ...succeeded\n");
	else              printf("  ...failed, tried it %d times\n", maxTryCount);
	return disConnected;
}

void selectExaminationType(const EXAMINATION_TYPES examination) 
{
	char msg[MAX_MSG_SIZE];
    snprintf(msg, MAX_MSG_SIZE, "%s%c%d", 
             EXAMINATION_MSG,     // "EXAM"
             MSG_ARGUMENT_SEPARATOR, // ':'
             examination);          // Number (bv. 0)

    // Send message
    writeMsgToSerialPort(msg);
}

bool getDoseDataFromCentralAcquisition(uint32_t * doseData)
{
	char msg[MAX_MSG_SIZE];
	if (getMsgFromCentralAcquisition(msg)) {
		*doseData = 666;// remove this line as soon as you are really doing something with msg
		return true;
	}
	return false;
}


typedef enum {
	WAITING_FOR_MSG_START_SYMBOL, 
	WAITING_FOR_MSG_END_SYMBOL
} MSG_RECEIVE_STATE;

static bool getMsgFromCentralAcquisition(char msg[MAX_MSG_SIZE])
{
    char receivedChar;
	int receiveIndex = 0;
	MSG_RECEIVE_STATE state = WAITING_FOR_MSG_START_SYMBOL;
	while (1) {
		if (readSerialPort(&receivedChar) == 1) {
			switch (state) {
			case WAITING_FOR_MSG_START_SYMBOL:
				if (receivedChar == MSG_START_SYMBOL) {
					receiveIndex = 0;
					state = WAITING_FOR_MSG_END_SYMBOL;
				}
				break;
			case WAITING_FOR_MSG_END_SYMBOL:
				if (receivedChar == MSG_END_SYMBOL) {
					msg[receiveIndex] = '\0';
					return true;
				}
				else msg[receiveIndex++] = receivedChar;   // oeps, oeps. 
				                                           // What will happen when receiveIndex equals MAX_MSG_SIZE.  FIX THIS
				break;
			default:
				break;
			}
		}
		else return false;
	}
}

static bool setupSerialConnection(void)
{
    // Dit is de poortnaam die je Mac gebruikt.
    // Pas deze naam aan als je Arduino een andere poort krijgt!
    char ttyName[] = "/dev/cu.usbmodem48CA435D02BC2"; 

	printf("Setting up serial connection...  ");
	
    if (setupSerialPort(ttyName) == 0) {	
        printf("  ...connected with %s\n", ttyName);
        return true;
    }
	
	printf("  ...failed to connect with %s\n", ttyName);
	printf("    	Did you connect the Arduino???\n");
    printf("    	Is the port name in CentralAcquisitionProxy.c correct?\n");
	return false;
}

static bool connect(void)
{
	char receivedMsg[MAX_MSG_SIZE];
	int tryCount = 0;
	const int maxTryCount = 10;
	bool connected = false;
	
	printf("Try to connect with CentralAquisition...  ");
	do {
		writeMsgToSerialPort(CONNECT_MSG);
		if (getMsgFromCentralAcquisition(receivedMsg)) {
			if (strcmp(receivedMsg, CONNECT_MSG) == 0) connected = true;
			else 									   tryCount++;			
		}
		else tryCount++;
		sleep(1);
	} while (!connected && tryCount < maxTryCount);
		
	if (connected) printf("  ...succeeded\n");
	else           printf("  ...failed, tried it %d times\n", maxTryCount);
	return connected;
}

static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE])
{
	writeSerialPort(MSG_START_SYMBOL);
	int i = 0;
	while (i < MAX_MSG_SIZE && msg[i] != '\0') {
		writeSerialPort(msg[i++]);
	}
	writeSerialPort(MSG_END_SYMBOL);
	return true;
}
