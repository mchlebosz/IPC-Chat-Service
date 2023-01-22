// REST API for communication with the server
//
#pragma once

typedef struct {
	int key;
	char* value;
} map_entry_t;

// create struct for message that contains the following:
// header:
//  - type:
#define TYPE_CODES 11
extern const map_entry_t typeCodes[TYPE_CODES];
//  - sender
//  - receiver
//  - time
//  - error code:
#define STATUS_CODES 24
extern const map_entry_t statusCodes[STATUS_CODES];
// body
#include "../message.h"
// get status code
char* getStatusCode(int key);

// get type code
char* getTypeCode(int key);

// login
Message APILogin(char* username, char* password);
// register
Message APIRegister(char* username, char* password);
// logout
Message APILogout(char* username);

// get the list of users
// get the list of messages
// get the list of groups

// send a message
// send a file
// send a request to add a user to the contact list
// send a request to add a user to a group
// send a request to create a group

// receive a message
// receive a file
