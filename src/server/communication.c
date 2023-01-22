#include "communication.h"

void serve(int* keep_running, int* msgid) {
	if (msgid < 0) {
		perror("msgget");
		exit(1);
	}

	while (keep_running) {
		Message msg;

		ssize_t msg_size = msgrcv(msgid, &msg, sizeof(msg), 0, 0);
		if (msg_size < 0) {
			perror("msgrcv");
			continue;
		}
		if (msg_size > sizeof(Message)) {
			printf(
				"Error: message size exceeds buffer size, discarding message\n");
			continue;
		}
		// handle the received message
		for (int i = 0; i < TYPE_CODES; i++) {
			if (msg.header.type == typeCodes[i].code) {
				printf("Received %s request from %s at %s: %s\n",
					   typeCodes[i].name, msg.header.sender, msg.header.time,
					   msg.body);
				break;
			}
		}

		// send response message
		msg.header.type       = 34;
		msg.header.statusCode = 200;
		msgsnd(msgid, &msg, sizeof(msg), 0);
	}
}