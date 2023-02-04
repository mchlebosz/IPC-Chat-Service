#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Message {
	long mtype;    // permission type
	// 11: server to server
	// 12: server to session
	// 13: server to client
	// 31: client to server
	// 32: client to session
	// 33: client to client
	// 21: session to server
	// 23: session to client
	// 22: session to session
	// mtype % 10 = receiver
	// mtype / 10 = sender

	struct mtext {
		struct header {
			short type;
			char sender[20];      // name or id
			char receiver[20];    // name or id
			char time[20];
			short statusCode;
		} header;
		char body[1000];
	} mtext;
} Message;

void msgInit(Message* msg, long permission, short type, const char* sender,
			 const char* receiver, short statusCode, const char* body);

void msgClear(Message* msg);

int receiveMessage(int* msgid, Message* msg, long permittedType);

int receiveMessageNoWait(int* msgid, Message* msg, long permittedType);