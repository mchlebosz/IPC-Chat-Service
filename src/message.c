#include "message.h"

void msgInit(Message* msg, long permission, short type, char* sender,
			 char* receiver, short statusCode, char* body) {
	msg->mtype             = permission;
	msg->mtext.header.type = type;
	strcpy(msg->mtext.header.sender, sender);
	strcpy(msg->mtext.header.receiver, receiver);
	strcpy(msg->mtext.header.time, "2023.12.31 23:59:59");
	msg->mtext.header.statusCode = 0;
	strcpy(msg->mtext.body, body);
}

void msgClear(Message* msg) {
	msg->mtype             = 0;
	msg->mtext.header.type = -1;
	strcpy(msg->mtext.header.sender, "");
	strcpy(msg->mtext.header.receiver, "");
	strcpy(msg->mtext.header.time, "");
	msg->mtext.header.statusCode = -1;
	strcpy(msg->mtext.body, "");
}