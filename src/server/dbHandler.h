// Handle file with database

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cJSON.h"

// Users and Groups structs
typedef struct User {
	int id;
	char name[32];
	char password[32];
	int groups[1000];
	int groupsCount;
	int friends[1000];
	int friendsCount;
	char publicKey[100];
} User;

typedef struct Group {
	int id;
	char name[32];
	char description[1000];
	int users[1000];
	int usersCount;
	char publicKey[100];
	char privateKey[100];
} Group;

// file handler
FILE* openDB(const char* fileName, const char* mode);

char* readFile(const char* filename);

int writeFile(const char* filename, const char* data);

int searchData(const char* fileName, const char* data);

int getUserByName(const char* file_name, User* user, char* name);

int getUserById(const char* file_name, User* user, int id);

int getAllUsers(const char* file_name, User** users, int* count);

int getGroupByName(const char* file_name, Group* group, char* name);

int getGroupById(const char* file_name, Group* group, int id);

int getAllGroups(const char* file_name, Group** groups, int* count);

int addUser(const char* file_name, User* user);

int addGroup(const char* file_name, Group* group);