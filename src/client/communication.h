// REST API for communication with the server
//
#pragma once

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils.h"
#include "../codes.h"
#include "../message.h"

// load data
void APIStart(void);
// connect to the server
int APICreateConnection(void);
// login
Message APILogin(const char* username, const char* password);
// register
Message APIRegister(const char* username, const char* password);
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
