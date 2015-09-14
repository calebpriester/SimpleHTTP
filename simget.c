//Name: Caleb Priester
//Date: 25 March 2015
//Asg:  HW3
//
//Description:  simget.c serves as a simple http client that sends http GET 
//  messages and parses the responses.  The file contents desired are either
//  printed to stdout or to a specified file.

#include "http.h"

//main() includes all logic for the simget client.
int main(int argc, char **argv) {

   char *url;
   char *httpServName;
   char *toGet;
   struct hostent *host;
   struct in_addr hostAddr;
   char *hostIP;
   int servPort = 8080;
   char *filename;
   FILE *output;
   int fileSpecified = 0;
   char httpMessage[10000];

   //Interpret data provided in the command line.
   if(argc%2 != 0) {
      DieWithUserMessage("Parameter(s)",
         "<URL> [-p <Port>] [-o <Filename>]");
   }

   url = argv[1];
   
   int i;
   for(i = 2; i < argc && argv[i][0] == '-'; i++) {
      if(argv[i][1] == 'p') servPort = atoi(argv[++i]);
      else if(argv[i][1] == 'o') {
         filename = argv[++i];
	 fileSpecified = 1;
      }
   }

   //If a filename was given, open that file.
   if(fileSpecified) output = fopen(filename, "w");

   httpServName = (char *)malloc(sizeof(url));
   toGet = (char *)malloc(sizeof(url));

   httpServName = strtok(url, "/");
   httpServName = strtok(NULL, "/");
   toGet = strtok(NULL, "\0");

   //Build the entire http request.
   strcpy(httpMessage, "GET /");
   strcat(httpMessage, toGet);
   strcat(httpMessage, " HTTP/1.1\r\nHost: ");
   strcat(httpMessage, httpServName);
   strcat(httpMessage, "\r\n\r\n");

   //Identify the IP of the desired http server using the provided URL.
   host = gethostbyname(httpServName);
   bcopy(host->h_addr, (char *)&hostAddr, sizeof(hostAddr));

   hostIP = inet_ntoa(hostAddr);

   //This section sets up a socket for communication and sends the http 
   //  request.
   int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(sock < 0) {
      DieWithSystemMessage("socket() failed");
   }

   struct sockaddr_in servAddr;
   memset(&servAddr, 0, sizeof(servAddr));
   servAddr.sin_family = AF_INET;
   int rtnVal = inet_pton(AF_INET, hostIP, &servAddr.sin_addr.s_addr);
   if(rtnVal == 0) {
      DieWithUserMessage("inet_pton() failed", "invalid address string");
   }
   else if(rtnVal < 0) {
      DieWithSystemMessage("inet_pton() failed");
   }
   servAddr.sin_port = htons(servPort);

   if(connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
      DieWithSystemMessage("connect() failed");
   }

   size_t httpMessageLen = strlen(httpMessage);

   ssize_t numBytes = send(sock, httpMessage, httpMessageLen, 0);
   if(numBytes < 0) {
      DieWithSystemMessage("send() failed");
   }
   else if(numBytes != httpMessageLen) {
      DieWithUserMessage("send()", "sent unexpected number of bytes");
   }

   //This loop is used to receive the http response and print the file contents
   //  provided.
   unsigned int totalBytesRcvd = 0;
   int firstIter = 1;
   while(1) {
      char buffer[1000000];

      numBytes = recv(sock, buffer, 1000000 - 1, 0);
      if(numBytes < 0) {
         DieWithSystemMessage("recv() failed");
      }
      if(numBytes == 0) break;
      totalBytesRcvd += numBytes;
      buffer[numBytes] = '\0';
      if(firstIter) {
         if(fileSpecified) fputs(strstr(buffer, "\r\n\r\n")+4, output);
         else fputs(strstr(buffer, "\r\n\r\n")+4, stdout);
         firstIter = 0;
      }
      else{
         if(fileSpecified) fputs(buffer, output);
         else fputs(buffer, stdout);
      }
   }

   fputc('\n', stdout);

   close(sock);
   exit(1);

}
