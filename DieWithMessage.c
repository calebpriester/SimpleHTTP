//Name: Caleb Priester
//Date: 25 March 2015
//Asg:  HW3
//
//Description:  DieWithMessage.c contains the logic for printing error messages
//  and exiting the program.

#include "http.h"

void DieWithUserMessage(const char *msg, const char *detail) {
   fputs(msg, stderr);
   fputs(": ", stderr);
   fputs(detail, stderr);
   fputc('\n', stderr);
   exit(0);
}

void DieWithSystemMessage(const char *msg) {
   perror(msg);
   exit(0);
}
