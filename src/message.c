#include "message.h"

void msgInit(Message* msg, short type, char* sender, char* receiver,
			 short statusCode, char* body) {
	msg->header.type = type;
	strcpy(msg->header.sender, sender);
	strcpy(msg->header.receiver, receiver);
	strcpy(msg->header.time, getTime());
	msg->header.statusCode = 0;
	strcpy(msg->body, body);
}

void msgClear(Message* msg) {
	msg->header.type = -1;
	strcpy(msg->header.sender, "");
	strcpy(msg->header.receiver, "");
	strcpy(msg->header.time, "");
	msg->header.statusCode = -1;
	strcpy(msg->body, "");
}