#include "chat.h"

#include <stdio.h>

#include "communication.h"

bool handleCommand(int);
void show_description(const char *username)
{
	printf("Hello, %s!\n", username);
	puts("Welcome to chat interface");
	puts("available commands:");
	puts("  1. open chat");
	puts("  2. online users");
	puts("  0. log out");
}

void show_chat_interface(const char *user, const char *token)
{
	show_description(user);

	bool running = true;

	while (running)
	{
		printf("> ");
		int cmd = scanfInt();
		running = handleCommand(cmd);
	}

	puts("You have been logged out!");
	// TODO
}

void online_users(void) 
{
	APIGetOnlineUsers();
}

void logout(void) 
{
	APILogout();
}

bool handleCommand(int cmd)
{
	switch (cmd)
	{
	case 1:
		// open chat
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