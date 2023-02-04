#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

// session type
typedef struct {
	char clientID[32];
	int sessionRunning;
	key_t sessionKey;
	int sessionQueue;
	int sessionPID;
	int userLoggedInID;
} Session;

// sessions list
typedef struct {
	int size;
	Session* sessions;
} Sessions;

// add session to sessions list
void addSession(Sessions* sessions, Session* session);

// remove session from sessions list
void removeSession(Sessions* sessions, char* clientID);

int getSessionQueue(Sessions* sessions, const char* clientID);

void session(int* keep_running, key_t sessionKey);

int openSession(int* sessionRunning, int* sessionQueue, char* clientID,
				int clientSeed, key_t* sessionKey, int* sessionPID);

char* generateKey();

int isSessionRunning(Sessions* sessions, const char* clientID);

int getSessionUserID(Sessions* sessions, const char* clientID, int* userID);

int setSessionUserID(Sessions* sessions, const char* clientID, int userID);