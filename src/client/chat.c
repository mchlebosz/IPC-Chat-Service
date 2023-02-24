#include "chat.h"

#include <stdio.h>

#include "communication.h"

char chattername[32];
#define DEBUGSERVER 0
#if DEBUGSERVER != 1
	#define _DEBUGSERVER_HELPER !
#else
	#define _DEBUGSERVER_HELPER
#endif


bool handleCommand(int);
void receiveMessagesLoop(void);

void show_description(const char *username) {
	printf("Hello, %s!\n", username);
	puts("Welcome to chat interface");
	puts("available commands:");
	puts("  1. open chat");
	puts("  2. online users");
	puts("  0. log out");
}

void show_chat_interface(const char* user, const char* token) {
	memset(chattername, 0, sizeof(chattername));
	show_description(user);

	pid_t pid = fork();
	if (pid) {
		setChildPID(pid);
	} else {
		setChildPID(getppid());
	}

	if (_DEBUGSERVER_HELPER pid) {
		receiveMessagesLoop();
		exit(0);
	}

	bool running = true;

	while (running) {
		printf("\n> ");
		int cmd = scanfInt();
		running = handleCommand(cmd);
	}
}

void chatLoop(void) {
	char* buff = NULL;
	size_t bytes_count = 0;
	if (getline(&buff, &bytes_count, stdin) <= 0) {
		kill(getChildPID(), SIGTERM);
		exit(-1);
	}
	while (true) {
		//memset(buff, 0, 512 * sizeof(char));
		printf("\n> ");
		fflush(stdout);
		if (getline(&buff, &bytes_count, stdin) <= 0) {
			kill(getChildPID(), SIGTERM);
			exit(-1);
		}	
		if (startsWith(buff, "/exit"))
			return;
		
		// check if input is a command anyways
		if (isCommand(buff)) {
			puts("invalid command!");
			continue;
		}
		// send text to receiver
		if (APIChatSendMessage(buff, chattername)
				.mtext.header.statusCode != 200) 
		{
			puts("Message hasn't been delivered!");
			return;
		}
	}
}

void closeChat(void) {
	puts("You've left the chat!");
	APIEndChat(chattername);
	memset(chattername, 0, sizeof(chattername));
}

void openChat(void) {
	printf("enter receiver's nickname:\n> ");
	fflush(stdout);
	char buff[64];
	memset(buff, 0, sizeof(buff));
	if (scanf("%s", buff) <= 0) {
		kill(getChildPID(), SIGTERM);
		exit(-1);
	}
	if (buff[31] != 0) { // probably overflow
		memset(buff + sizeof(buff) / 2 - 1, 0, sizeof(buff) / 2);
	}

	Message result = APIBeginChat(buff);
	if (result.mtext.header.statusCode != 200) { // user does not exist! 
		printf("\nthis user is not online!\n\n> ");					// (or any other error...)
		fflush(stdout);
		return; 
	}

	strcpy(chattername, buff);
	printf("\nYou are now chatting with %s!\n\n> ", chattername);
	fflush(stdout);
	chatLoop();
	closeChat();
}

void online_users(void)  {
	Message result = APIGetOnlineUsers();
	if (result.mtext.header.statusCode != 200) {
		puts("Something went wrong...");
		return;
	}
	char* data = result.mtext.body;

	puts("Available users:");
	const char* next = strtok(data, ";");

	for (int i = 0; next; ++i) {
		printf("  %d. %s\n", i + 1, next);
		next = strtok(NULL, ";");
	}
	puts("");
}

void logout(void)  {
	APILogout();
	puts("You've been logged out!");
}

bool handleCommand(int cmd) {
	switch (cmd)
	{
	case 1:
		// open chat
		openChat();
		break;
	case 2:
		// get online users
		online_users();
		break;
	case 0:
		// log out and exit interface
		logout();
		return false;
	default:
		puts("invalid command!");
	}
	return true;
}

void receiveMessagesLoop(void) {
	char currentChatter[32];
	while (true) {
		Message result = APIChatReceiveMessage();
		int status = result.mtext.header.statusCode;
		if (status != 200) exit(0);
		int type = result.mtext.header.type;
		if (type == 100) { // waiting for receiver to join the chat
			strcpy(currentChatter, result.mtext.body);
			continue;
		} 
		if (type == 101) { // receiver left the chat
			memset(currentChatter, 0, sizeof(currentChatter));
			continue;
		}

		const char* data   = result.mtext.body;
		const char* time   = result.mtext.header.time;
		const char* sender = result.mtext.header.sender;

		if (isChatBeginMsg(data)) {
			if (strcmp(currentChatter, sender) != 0) {
				printf("\n%s is trying to reach you\n\n> ", sender);
			} else
				printf("\n%s joined the chat!\n\n> ", sender);
		} else if (isChatEndMsg(data)) {
			if (strcmp(currentChatter, sender) != 0) continue;
			printf("\n%s left the chat!\n\n> ", sender);
			memset(currentChatter, 0, sizeof(currentChatter));
		} else {
			if (strcmp(currentChatter, sender) == 0) {
				printf("(%s) %s~# %s\n\n> ", time + 11, sender, data);
			}
		}
		fflush(stdout);
	}
}