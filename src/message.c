#include "message.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "codes.h"

void msgInit(Message* msg, long permission, short type, const char* sender,
			 const char* receiver, short statusCode, const char* body) {
	time_t t 	 = time(NULL);
  	struct tm tm = *localtime(&t);

	msg->mtype             = permission;
	msg->mtext.header.type = type;
	strcpy(msg->mtext.header.sender, sender);
	strcpy(msg->mtext.header.receiver, receiver);
	char time[128];
	sprintf(time, "%04d.%02d.%02d %02d:%02d:%02d", 
		(tm.tm_year + 1900), (tm.tm_mon + 1), tm.tm_mday, 
		tm.tm_hour, tm.tm_min, tm.tm_sec
		);
	memcpy(msg->mtext.header.time, time, 20);
	msg->mtext.header.statusCode = statusCode;
	printf("[prebody]%s[/body]\n", body);
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