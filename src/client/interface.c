//create login and register functions
//create a function to show the interface
//create a function to show the menu
//create a function to show the chat interface

#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clientLogin(void){
    char username[30];
    char password[30];
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);
    //send username and password to server
    //receive response from server
    //if response is success, show chat interface
    //else, show error message
}

void clientRegister(void){
    char username[30];
    char password[30];
    char confirmPassword[30];
    printf("Username: ");
    scanf("%s", username);

    //info about password requirements
    while (1)
    {
        //reset password and confirm password to empty string
        memset(password, 0, sizeof(password));
        memset(confirmPassword, 0, sizeof(confirmPassword));

        printf("Password must contain at least 8 characters, maximum of 29 characters,\n 1 uppercase letter, 1 lowercase letter, 1 number and 1 special character.\n");
        printf("Password: ");
        scanf("%s", password);
        printf("Confirm password: ");
        scanf("%s", confirmPassword);
    //check if password and confirm password are the same
    //if not, show error message
        if (strcmp(password, confirmPassword) != 0)
        {
            printf("Password and confirm password are not the same. Please try again.\n");
        }
        else
        {
            break;
        }
    }

    //send username and password to server
    //receive response from server
    //if response is success, show chat interface
    //else, show error message
}

void showInterface(void){
    printf("Welcome");
    while (1)
    {
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Exit\n");
        printf("Please choose one of the options: ");
        int option;
        scanf("%d", &option);
        switch (option)
        {
        case 1:
            clientLogin();
            break;
        case 2:
            clientRegister();
            break;
        case 3:
            exit(0);
            break;
        default:
            printf("Invalid option. Please try again.\n");
            break;
        }
    }

}