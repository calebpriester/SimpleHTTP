//Name: Caleb Priester
//Date: 25 March 2015
//Asg:  HW3
//
//Description:  simhttp.c serves as a smple http server that accepts GET and 
//  HEAD requests from http clients.  The server is able to detect basic errors
//  (including 400, 403, 404, and 405 errors), and can build an appropriate
//  http responses.

#include "http.h"

static const int MAXPENDING = 5;
int servPort;
char *dirName;
FILE *dir;

//main() contains the logic for processing the data provided in the command 
//  line and sets up a TCP socket to accept clients.  There is a separate 
//  method for handling the clients once a connection is accepted.
int main(int argc, char *argv[]) {
   servPort = 8080;
   dirName = ".";

   //Process data given in the command line.
   if(argc != 1) {
      if(argv[1][0] == '-') {
         servPort = atoi(argv[2]);
	 if(argc == 4) dirName = argv[3];
      }
      else dirName = argv[1];
   }

/* -- This section sets up the TCP socket and prepares to accept clients -- */
   int servSock;
   if((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      DieWithSystemMessage("socket() failed");
   }

   struct sockaddr_in servAddr;
   memset(&servAddr, 0, sizeof(servAddr));
   servAddr.sin_family = AF_INET;
   servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servAddr.sin_port = htons(servPort);

   if(bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
      DieWithSystemMessage("bind() failed");
   }

   if(listen(servSock, MAXPENDING) < 0) {
      DieWithSystemMessage("listen() failed");
   }

   for(;;) {
      struct sockaddr_in clntAddr;
      socklen_t clntAddrLen = sizeof(clntAddr);

      int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
      if(clntSock < 0) {
         DieWithSystemMessage("accept() failed");
      }

      char clntName[INET_ADDRSTRLEN];
      if(inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) == NULL)
         puts("Unable to get client address");

      HandleTCPClient(clntSock);
   }
}

//HandleTCPClient() contains the logic for receiving an http message, parsing 
//  the message, and writing a correct response.
void HandleTCPClient(int clntSocket) {
   char *buffer = malloc(1000000);
   char *body = malloc(1000000);
   char *modifiedBody = malloc(1000000);
   int recvMsgSize;
   int isGet = 1;
   char *httpMessage = malloc(1000000);
   int fileLen = 0;
   char *length = malloc(50);
   char *httpMessageBody = malloc(1000000);
   char ch;
   int errorHasOccurred = 0;
   int error400HasOccurred = 0;
   char *date = malloc(1000);
   char *lmdate = malloc(1000);
   time_t myTime;
   struct tm tm;
   struct stat status;
   char *toGet;
   char *indicator = malloc(3);

   //Receive the message from the client.
   if((recvMsgSize = recv(clntSocket, buffer, 1000000, 0)) < 0)
      DieWithSystemMessage("recv() failed");

   //Start building http response.
   strcat(body, buffer);
   strcat(httpMessage, "HTTP/1.1 ");

   //Check for 405 error.
   if(strstr(body, "GET") == NULL ||
      strcmp(body, strstr(body, "GET")) != 0) {
      if (strstr(body, "HEAD") == NULL ||
         strcmp(body, strstr(body, "HEAD")) != 0) {
      
         strcat(httpMessage, "405 Method Not Allowed\r\nAllow: HEAD, GET\r\n");
	 strcpy(indicator, "405");
         errorHasOccurred = 1;
      }
      else isGet = 0;
   }
   //Check for 400 error.
   else if(strstr(body, "HTTP/1.1") == NULL) {
      strcat(httpMessage, "400 Bad Request\r\n");
      strcpy(indicator, "400");
      errorHasOccurred = 1;
      error400HasOccurred = 1;
   }

   //If no 400 error, get the name of the desired file.
   if(!error400HasOccurred) {
      strcpy(modifiedBody, body);
      strtok(modifiedBody, " ");
      toGet = strtok(NULL, " ");
      
      //Check for 403 error.
      if(strstr(toGet, "..") != NULL) {
         strcat(httpMessage, "403 Forbidden\r\n");
         strcpy(indicator, "403");
         errorHasOccurred = 1;
      }
   }

   //Check for file-related errors.
   if(!errorHasOccurred) {
      dir = fopen(concat(dirName, toGet), "r");
      if(dir == NULL) {
         //Check for 404 error.
         if(errno == 2) {
	    strcat(httpMessage, "404 Not Found\r\n");
	    strcpy(indicator, "404");
	    errorHasOccurred = 1;
	 }
	 //Check for 403 error.
	 else if(errno == 13) {
	    strcat(httpMessage, "403 Forbidden\r\n");
	    strcpy(indicator, "403");
	    errorHasOccurred = 1;
	 }
      }
      //If no errors, prepare a 200 OK message.
      else {
         strcat(httpMessage, "200 OK\r\n");
	 strcpy(indicator, "200");
      }
   }
   
   //Add Connection and Server headers to http response.
   strcat(httpMessage, "Connection: close\r\n");
   strcat(httpMessage, "Server: simhttp/1.0\r\n");

   //Add Date header to response.
   strcat(httpMessage, "Date: ");
   time(&myTime);
   tm = *localtime(&myTime);
   strftime(date, 1000, "%a, %d %b %Y %H:%M:%S", &tm);
   strcat(httpMessage, date);
   strcat(httpMessage, "\r\n");

   //Correctly format date for server output.
   strftime(date, 1000, "%a %b %Y %H:%M", &tm);
   
   //If no error occurred, add Last-Modified header. 
   if(!errorHasOccurred) {
      strcat(httpMessage, "Last-Modified: ");
      stat(concat(dirName, toGet), &status);
      tm = *localtime(&(status.st_mtime));
      strftime(lmdate, 1000, "%a, %d %b %Y %H:%M:%S", &tm);
      strcat(httpMessage, lmdate);
      strcat(httpMessage, "\r\n");

      //Add Content -Type header.
      strcat(httpMessage, "Content -Type: ");
      if(strcmp(strrchr(toGet, '.'), ".html") == 0 ||
         strcmp(strrchr(toGet, '.'), ".htm") == 0) {
         strcat(httpMessage, "text/html\r\n");
      }
      else if(strcmp(strrchr(toGet, '.'), ".css") == 0) {
         strcat(httpMessage, "text/css\r\n");
      }
      else if(strcmp(strrchr(toGet, '.'), ".js") == 0) {
         strcat(httpMessage, "application/javascript\r\n");
      }
      else if(strcmp(strrchr(toGet, '.'), ".txt") == 0) {
         strcat(httpMessage, "text/plain\r\n");
      }
      else if(strcmp(strrchr(toGet, '.'), ".jpg") == 0) {
         strcat(httpMessage, "image/jpeg\r\n");
      }
      else if(strcmp(strrchr(toGet, '.'), ".pdf") == 0) {
         strcat(httpMessage, "application/pdf\r\n");
      }
      else {
         strcat(httpMessage, "application/octet-stream\r\n");
      }
      
      //If a GET method was called, copy the file contents to add to response.
      if(isGet) {
         while((ch = fgetc(dir)) != EOF) {
            httpMessageBody[fileLen] = ch;
	    fileLen++;
         }
         httpMessageBody[fileLen] = '\0';

         sprintf(length, "%d", fileLen);

	 //Add Content -Length header
         strcat(httpMessage, "Content -Length: ");
         strcat(httpMessage, length);
         strcat(httpMessage, "\r\n\r\n");
         strcat(httpMessage, httpMessageBody);
      }
      else strcat(httpMessage, "\r\n");
   }
   else strcat(httpMessage, "\r\n");

   //Print server output to stdout.
   if(error400HasOccurred) 
      printf("%s\t%s\n", date, indicator);
   else
      printf("%s\t%s\t%s\t%s\n", modifiedBody, toGet+1, date, indicator);

   //Send the http response.
   ssize_t numBytes = send(clntSocket, httpMessage, strlen(httpMessage), 0);
   if(numBytes < 0) {
      DieWithSystemMessage("send() failed");
   }
   else if(numBytes != strlen(httpMessage)) {
      DieWithUserMessage("send()", "sent unexpected number of bytes");
   }

   close(clntSocket);

}

//concat() allows for the concatenation of two strings without modifying the 
//  original string.  This is used for identifying the specified file without
//  modifying the variable for server directory.
char* concat(char *s1, char *s2) {
   char *result = malloc(strlen(s1)+strlen(s2)+1);
   strcpy(result, s1);
   strcat(result, s2);
   return result;
}
