#pragma once

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include "../codes.h"
#include "../message.h"
#include "../utils.h"
#include "dbHandler.h"
#include "session.h"

void serve(int* keep_running, int* msgid, char* db);

int registerUser(char* username, char* password, char** key, char* db);

int loginUser(char* username, char* password, char** key, char* db);

char* getOnlineUsersID(Sessions sessions, const char* db);

char* getOnlineClientsID(Sessions sessions);

char* getUsernamebyClientID(Sessions sessions, char* clientID, const char* db);

char* getClientIDbyUsername(Sessions sessions, char* username, const char* db);