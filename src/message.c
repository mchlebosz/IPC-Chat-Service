#include "message.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "codes.h"

/**
 * It initializes a message
 *
 * @param msg The message to be initialized.
 * @param permission the permission number of the message
 * @param type the type of the message
 * @param sender the sender of the message
 * @param receiver the name of the user who will receive the message
 * @param statusCode the status code of the message
 * @param body the message body
 */
void msgInit(Message* msg, long permission, short type, const char* sender,
			 const char* receiver, short statusCode, const char* body) {
	time_t t     = time(NULL);
	struct tm tm = *localtime(&t);

	msg->mtype             = permission;
	msg->mtext.header.type = type;
	strcpy(msg->mtext.header.sender, sender);
	strcpy(msg->mtext.header.receiver, receiver);
	char time[128];
	sprintf(time, "%04d.%02d.%02d %02d:%02d:%02d", (tm.tm_year + 1900),
			(tm.tm_mon + 1), tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	memcpy(msg->mtext.header.time, time, 20);
	msg->mtext.header.statusCode = statusCode;
	printf("[prebody]%s[/body]\n", body);
	strcpy(msg->mtext.body, body);
}

/**
 * It clears the message
 *
 * @param msg The message to be cleared.
 */
void msgClear(Message* msg) {
	msg->mtype             = 0;
	msg->mtext.header.type = -1;
	strcpy(msg->mtext.header.sender, "");
	strcpy(msg->mtext.header.receiver, "");
	strcpy(msg->mtext.header.time, "");
	msg->mtext.header.statusCode = -1;
	strcpy(msg->mtext.body, "");
}

/**
 * It receives a message from the message queue, and prints it to the console
 *
 * @param msgid the message queue id
 * @param msg the message to be sent
 * @param permittedType the type of message to receive. If you want to receive
 * any message, use 0.
 * @param nowait if true, the function will return immediately if there are no
 * messages in the queue.
 *
 * @return The return value is the status code of the message.
 */
int _receiveMessage(int* msgid, Message* msg, long permittedType, bool nowait) {
	ssize_t msg_size = msgrcv(*msgid, msg, sizeof(*msg), permittedType,
							  nowait ? IPC_NOWAIT : 0);

	if (msg_size < 0) {
		if (!nowait) perror("msgrcv");
		return 500;
	}
	if (msg_size > sizeof(Message)) {
		printf("Error: message size exceeds buffer size, discarding message\n");
		return 413;
	}

	// print msg
	printf("Received message:\n");
	printf("mtype: %ld\n", msg->mtype);
	printf("type: %d\n", msg->mtext.header.type);
	printf("body: %s\n", msg->mtext.body);
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

/**
 * It receives a message from the message queue with the given ID, and stores it
 * in the given Message struct
 *
 * @param msgid The message queue ID.
 * @param msg The message to be sent.
 * @param permittedType If this is set to -1, then the message will be received
 * regardless of its type. If it is set to a positive number, then the message
 * will only be received if its type matches the permittedType.
 *
 * @return The return value is the message type.
 */
int receiveMessageNoWait(int* msgid, Message* msg, long permittedType) {
	return _receiveMessage(msgid, msg, permittedType, true);
}

/**
 * It receives a message from the message queue with the given ID, and stores it
 * in the given Message struct
 *
 * @param msgid The message queue ID.
 * @param msg The message to be received.
 * @param permittedType The type of message you want to receive. If you want to
 * receive any message, pass 0.
 *
 * @return The return value is the message type.
 */
int receiveMessage(int* msgid, Message* msg, long permittedType) {
	return _receiveMessage(msgid, msg, permittedType, false);
}