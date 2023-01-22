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

// session type
typedef struct {
	int sessionRunning;
	key_t sessionKey;
	int sessionQueue;
} Session;

// sessions list
typedef struct {
	int size;
	Session* sessions;
} Sessions;

// add session to sessions list
void addSession(Sessions* sessions, Session session);

// remove session from sessions list
void removeSession(Sessions* sessions, key_t sessionKey);

void serve(int* keep_running, int* msgid, FILE* db);

int registerUser(char* username, char* password);

int openSession(int* sessionRunning, int* sessionQueue, key_t clientKey);
