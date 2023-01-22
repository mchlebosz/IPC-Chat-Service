# IPC-Chat-Service
Parallel and System programing project

## IMPORTANT!
When commiting to main (and other branches too) please follow given structure:
[Commit Guidelines](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#-git-commit-guidelines)

## About
using C11
version i dont know

## Codes
```c
const map_entry_t typeCodes[TYPE_CODES] = {
	{ 10, "Login" },
	{ 11, "Register" },
	{ 12, "Logout" },
	{ 13, "Get list of users" },
	{ 14, "Get list of messages" },
	{ 15, "Get list of groups" },
	{ 24, "Send a message" },
	{ 23, "Send a request to add a user to the contact list" },
	{ 25, "Send a request to add a user to a group" },
	{ 26, "Send a request to create a group" },
	{ 34, "Receive a message" }
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

## MVP

### Client
- [x] Login
- [ ] Send a message


### Server
- [] Login
- [] Register
- [] Message

