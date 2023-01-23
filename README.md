# IPC-Chat-Service
Parallel and System programing project

## IMPORTANT!
When commiting to main (and other branches too) please follow given structure:
[Commit Guidelines](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#-git-commit-guidelines)

## Stack
using C11
For building project use Makefile

## How to run
1. Clone repository
2. Go to `.` directory
3. Run `make` command
4. Run `./bin/server` command
5. Run `./bin/client` command
6. Enjoy!

## About
This is a project for Parallel and System programing course at PUT. The goal is to create a chat service using IPC (Inter Process Communication) methods. We are using C11 standard for programming. We are using Makefile for building project. \
[Original project description](https://www.cs.put.poznan.pl/akobusinska/downloads/projekt2022.pdf) \

Assumptions for this project are:
- Creating a chat service using IPC Queue
- 2 programs: client and server
- Client can send messages to any user or group
- All messages are need to go through server

Account:
- [x] Login
- [ ] Logout
- [x] Register
- [x] Send a message to a user
- [ ] Send a message to a group
- [ ] See who is online
- [ ] See who is in a group


Group:
- [ ] Create a group
- [ ] Add a user to a group
- [ ] See who is in a group
- [ ] Remove a user from a group
- [ ] See all groups

Our own weir ideas:
- [ ] Message encryption
- [ ] Message confirmation
- [ ] Message history


## Codes
File with codes is in `src/codes.c`. It contains all codes used in project. \
```c
const map_entry_t typeCodes[TYPE_CODES] = {
	{ 0, "Connection" },
	{ 1, "Response" },
	{ 10, "Login" },
	{ 11, "Register" },
	{ 12, "Logout" },
	{ 13, "Get list of users" },
	{ 14, "Get list of messages" },
	{ 15, "Get list of groups" },
	{ 24, "Send a message to User" },
	{ 23, "Send a request to add a user to the contact list" },
	{ 25, "Send a request to add a user to a group" },
	{ 26, "Send a request to create a group" },
	{ 27, "Send a message to Group" },
	{ 34, "Receive a message from user" },
	{ 37, "Receive a message from group" },
};


const map_entry_t statusCodes[STATUS_CODES] = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 413, "Request Entity Too Large" },
	{ 418, "I'm a teapot" },
	{ 429, "Too Many Requests" },
	{ 440, "Login timeout" },
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 503, "Service Unavailable" }
};
```
## Files
- `src/codes.c` - file with codes
- `src/message.c` - file with message structure and functions
- `src/client/main.c` - client main file
- `src/server/main.c` - server main file
- `src/server/serverQueue.c` - server functions and main loop
- `src/server/dbHandler.c` - file handling
- `src/server/session.c` - file with session structure and functions




## Authors
- [Mateusz Chlebosz](https://github.com/mchlebosz)
- [Jakub Aszyk](https://github.com/kubsnn)