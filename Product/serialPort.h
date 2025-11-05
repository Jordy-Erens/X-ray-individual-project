#ifndef SERIALPORT_H
#define SERIALPORT_H
int setupSerialPort(char* ttyName);
int closeSerialPort(void);
int readSerialPort(char* receivedChar);
int writeSerialPort(char writeChar);

#endif
