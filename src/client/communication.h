// REST API for communication with the server
//
#pragma once

#include "../codes.h"
#include "../message.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int clientQueueID;

// connect to the server
void APICreateConnection(void);
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
