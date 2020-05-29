#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
/* You will to add includes here */

#define PORT 4950
#define MAXDATASIZE 256

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
  switch (sa->sa_family)
  {
  case AF_INET:
    inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
              s, maxlen);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
              s, maxlen);
    break;

  default:
    strncpy(s, "Unknown AF", maxlen);
    return NULL;
  }

  return s;
}

#include <calcLib.h>

using namespace std;

int main(int argc, char *argv[])
{
  int listenfd;
  int connfd;
  int numbytes;

  double result;

  struct sockaddr_in serverAddress;
  struct sockaddr_in clientAddress;
  struct sockaddr_storage their_addr;

  char *ptr;

  char s[INET6_ADDRSTRLEN];
  char msg[MAXDATASIZE];
  char buf[MAXDATASIZE];

  socklen_t clientAddressLength;

  string arith[] = {"add", "div", "mul", "sub", "fadd", "fdiv", "fmul", "fsub"};

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(PORT);

  bind(listenfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  listen(listenfd, 1);
  printf("Listening on port %d \n", PORT);
  while (1)
  {
    printf("\n*****server waiting for new client connection:*****\n");
    clientAddressLength = sizeof(clientAddress);
    connfd = accept(listenfd, (struct sockaddr *)&clientAddress, &clientAddressLength);
    printf("accept = %d \n", connfd);
    printf("listener: got packet from %s:%d\n",
           inet_ntop(clientAddress.sin_family,
                     get_in_addr((struct sockaddr *)&clientAddress),
                     s, sizeof s),
           ntohs(clientAddress.sin_port));
    printf("Send header to Client\n");
    send(connfd, "TEXT TCP 1.0\n", sizeof("TEXT TCP 1.0\n"), 0);
    numbytes = recv(connfd, buf, MAXDATASIZE, 0);
    string res;
    for (int i = 0; i <= numbytes; i++)
    {
      if (buf[i] == '\n')
      {
        res.assign(buf, buf + i);
        break;
      }
    }
    if (res == "OK")
    {
      printf("Receive OK from Client\n");
      initCalcLib();
      ptr = randomType();
      char s[MAXDATASIZE];
      string option;
      if (ptr[0] == 'f')
      {
        int index = 4 + rand() % 4;
        option = arith[index];
        double f1 = randomFloat();
        double f2 = randomFloat();
        while (f2 == 0.0f)
        {
          f2 = randomFloat();
        }
        switch (index)
        {
        case 4:
          result = f1 + f2;
          break;
        case 5:
          result = f1 / f2;
          break;
        case 6:
          result = f1 * f2;
          break;
        case 7:
          result = f1 - f2;
          break;
        }
        sprintf(s, "%s %8.8g %8.8g\n", option.c_str(), f1, f2);
      }
      else
      {
        int index = rand() % 4;
        option = arith[index];
        int i1 = randomInt();
        int i2 = randomInt();
        while (i2 == 0)
        {
          i2 = randomInt();
        }
        switch (index)
        {
        case 0:
          result = i1 + i2 + 0.0f;
          break;
        case 1:
          result = i1 / i2 + 0.0f;
          break;
        case 2:
          result = i1 * i2 + 0.0f;
          break;
        case 3:
          result = i1 - i2 + 0.0f;
          break;
        }
        sprintf(s, "%s %d %d\n", option.c_str(), i1, i2);
      }
      printf("Send option %s to Client\n", s);
      send(connfd, s, sizeof(s), 0);
      numbytes = recv(connfd, msg, MAXDATASIZE, 0);
      printf("Receive result %s from Client\n", msg);
      printf("My result is %f\n", result);
      if (fabs(stod(msg) - result) < 0.00001)
      {
        send(connfd, "OK", sizeof("OK"), 0);
      }
      else
      {
        send(connfd, "ERROR", sizeof("ERROR"), 0);
      }
      close(connfd);
    }
  }
  return 0;
}