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
	char* sessionID;
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
void removeSession(Sessions* sessions, char* sessionID);

int getSessionQueue(Sessions* sessions, char* sessionID);

void serve(int* keep_running, int* msgid, char* db);

int registerUser(char* username, char* password, char** key, char* db);

int openSession(int* sessionRunning, int* sessionQueue, char* clientID,
				int clientSeed);

int receiveMessage(int* msgid, Message* msg, long permittedType);

char* generateKey();

int loginUser(char* username, char* password, char** key, char* db);