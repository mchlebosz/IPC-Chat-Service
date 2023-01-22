#include "message.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "codes.h"

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

int receiveMessage(int* msgid, Message* msg, long permittedType) {
	ssize_t msg_size = msgrcv(*msgid, msg, sizeof(msg), permittedType, 0);
	if (msg_size < 0) {
		perror("msgrcv");
		return 500;
	}
	if (msg_size > sizeof(Message)) {
		printf("Error: message size exceeds buffer size, discarding message\n");
		return 413;
	}
	// handle the received message
	for (int i = 0; i < TYPE_CODES; i++) {
		if (msg->mtext.header.type == typeCodes[i].code) {
			printf("Received %s request from %s at %s: %s\n", typeCodes[i].name,
				   msg->mtext.header.sender, msg->mtext.header.time,
				   msg->mtext.body);
			break;
		}
	}
	return 200;
}