//Name: Caleb Priester
//Date: 25 March 2015
//Asg:  HW3
//
//Description:  http.h has all include statements and and method prototypes
//  needed for simhttp.c, simget.c, and DieWithMessage.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

void DieWithUserMessage(const char *msg, const char *detail);
void DieWithSystemMessage(const char *msg);

void HandleTCPClient(int clntSocket);
char* concat(char *s1, char *s2);
