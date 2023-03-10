#include "utils.h"

/**
 * It takes a string and returns a hash value.
 *
 * @param str The string to hash
 */
unsigned long hash(const char *str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return abs((long)hash);
}

/**
 * It takes a clientId and a sessionSeed and returns a sessionKey
 *
 * @param clientId The client ID of the client that is connecting to the server.
 * @param sessionSeed A random number generated by the client.
 *
 * @return The clientId and sessionSeed are being shifted 7 bits to the left and
 * then added together.
 */
int createSessonKey(int clientId, int sessionSeed) {
	return abs((clientId << 7) + sessionSeed);
}

bool isNumber(const char* s) {
	while (*s) {
		if (!('0' <= *s && *s <= '9')) return false;
		s++;
	}
	return true;
}

int scanfInt(void) {
	char buff[64];
	do {
		memset(buff, 0, sizeof(buff));
		if (scanf("%s", buff) != 1) {
			exit(-1);
		}
		if (isNumber(buff))
			break;
		else
			printf("not a number!  try again\n> ");
	} while (true);
	return atoi(buff);
}

bool startsWith(const char* s, const char* flag) {
	while (*s && *flag) {
		if (*s != *flag) return false;
		s++;
		flag++;
	}
	return !*flag;
}

bool isCommand(const char* s) {
	return startsWith(s, "/");
}

bool isChatBeginMsg(const char *msg) {
	const char _msg[] = notifyChatBeginMsg;
    return memcmp(msg, notifyChatBeginMsg, sizeof(_msg) - 1) == 0;
}

bool isChatEndMsg(const char *msg) {
	const char _msg[] = notifyChatEndMsg;
    return memcmp(msg, notifyChatEndMsg, sizeof(_msg) - 1) == 0;
}

char* trim(char* s) {
    while (isspace(*s)) ++s;
    char* end = &s[strlen(s) - 1];
    while (end > s && isspace(*end)) --end;
    end[1] = '\0';
    return s;
}

int childPID = 0;

void setChildPID(int pid) {
	childPID = pid;
}

int getChildPID() {
	return childPID;
}