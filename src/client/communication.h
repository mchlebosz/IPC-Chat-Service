// REST API for communication with the server
//
#pragma once

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include "../codes.h"
#include "../message.h"
#include "../utils.h"

// connect to the server
int APICreateConnection(void);

void APIStart(void);

// login
Message APILogin(const char* username, const char* password);
// register
Message APIRegister(const char* username, const char* password);
// logout
Message APILogout(void);
// get the list of users
Message APIGetOnlineUsers(void);

Message APIBeginChat(const char* username);

Message APIChatSendMessage(const char* data);

Message APIChatReceiveMessage(void);

Message APIEndChat(void);
// get the list of messages
// get the list of groups

// send a message
// send a file
// send a request to add a user to the contact list
// send a request to add a user to a group
// send a request to create a group

// receive a message
// receive a file
