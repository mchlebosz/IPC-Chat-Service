#pragma once

void showInterface(void);

// login function
void clientLogin(void);

// register function
void clientRegister(void);

// check username
short checkUsername(char* username);

// check password
short checkPassword(char* password, char* confirmPassword);
