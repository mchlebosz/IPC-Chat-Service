#include "chat.h"

#include <stdio.h>

// #include "communication.h"

void show_description(const char *username)
{
	printf("Hello, %s!\n", username);
	puts("Welcome to chat interface");
	puts("available commands:");
	puts("  1. add friend");
	puts("  2. open chat");
}

void handleCommand(int cmd)
{
	switch (cmd)
	{
	case 1:
		// add friend
		break;
	case 2:
		// open chat
		break;
	default:
		puts("invalid command!");
	}
}

void show_chat_interface(const char *user, const char *token)
{
	show_description(user);

	while (true)
	{
		printf("> ");
		int cmd = scanfInt();
		handleCommand(cmd);
	}

	// TODO
}