#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

// session type
typedef struct {
	char* sessionID;
	int sessionRunning;
	key_t sessionKey;
	int sessionQueue;
	int sessionPID;
} Session;

// sessions list
typedef struct {
	int size;
	Session* sessions;
} Sessions;

// add session to sessions list
void addSession(Sessions* sessions, Session session);

// remove session from sessions list
void removeSession(Sessions* sessions, char* sessionID);

int getSessionQueue(Sessions* sessions, char* sessionID);

void session(int* keep_running, key_t sessionKey);

int openSession(int* sessionRunning, int* sessionQueue, char* clientID,
				int clientSeed, key_t* sessionKey, int* sessionPID);

char* generateKey();