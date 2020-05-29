#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>
#include <iterator>

#include <arpa/inet.h>
#define PORT "4950"     // the port client will be connecting to
#define MAXDATASIZE 256 // max number of bytes we can get at once

// Included to get the support library
#include <calcLib.h>

using namespace std;

vector<string> split(char *str, const char *delim)
{
  vector<string> elems;
  char *s = strtok(str, delim);
  while (s != NULL)
  {
    elems.push_back(s);
    s = strtok(NULL, delim);
  }
  return elems;
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
  int sockfd, numbytes;
  int rv;

  struct addrinfo hints, *servinfo, *p;

  char buf[MAXDATASIZE];
  char result[17];

  if (argc != 2)
  {
    fprintf(stderr, "usage: client hostname\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1)
    {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd);
      perror("client: connect");
      continue;
    }

    break;
  }
  if (p == NULL)
  {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }
  numbytes = recv(sockfd, buf, MAXDATASIZE, 0);
  if (numbytes == -1)
  {
    perror("recv");
    exit(1);
  }
  if (numbytes == 0)
  {
    printf("Server closed.\n");
  }
  string res;
  for (int i = 0; i <= numbytes; i++)
  {
    if (buf[i] == '\n')
    {
      res.assign(buf, buf + i);
      break;
    }
  }
  printf("Receive %s from Server\n", res.c_str());
  if (res == "TEXT TCP 1.0")
  {
    printf("Send OK to Server\n");
    send(sockfd, "OK\n", 4, 0);
    numbytes = recv(sockfd, buf, MAXDATASIZE, 0);
    char message[numbytes];
    for (int i = 0; i <= numbytes; i++)
    {
      if (buf[i] == '\n')
      {
        memcpy(&message, buf, i);
        break;
      }
    }
    printf("Receive option %s from Server\n", message);
    std::vector<std::string> list = split(message, " ");
    string option = list[0];
    string num1 = list[1];
    string num2 = list[2];
    if (option[0] == 'f')
    {
      if (option == "fadd")
      {
        sprintf(result, "%8.8g\n", stod(num1) + stod(num2));
      }
      else if (option == "fdiv")
      {
        sprintf(result, "%8.8g\n", stod(num1) / stod(num2));
      }
      else if (option == "fmul")
      {
        sprintf(result, "%8.8g\n", stod(num1) * stod(num2));
      }
      else if (option == "fsub")
      {
        sprintf(result, "%8.8g\n", stod(num1) - stod(num2));
      }
    }
    else
    {
      if (option == "add")
      {
        sprintf(result, "%dg\n", stoi(num1) + stoi(num2));
      }
      else if (option == "div")
      {
        sprintf(result, "%d\n", stoi(num1) / stoi(num2));
      }
      else if (option == "mul")
      {
        sprintf(result, "%d\n", stoi(num1) * stoi(num2));
      }
      else if (option == "sub")
      {
        sprintf(result, "%d\n", stoi(num1) - stoi(num2));
      }
    }
    printf("Send my result %s to Server\n", result);
    send(sockfd, result, sizeof(result), 0);
    numbytes = recv(sockfd, buf, MAXDATASIZE, 0);
    printf("My result to the option is %s\n", buf);
    sleep(0.5);
  }
  close(sockfd);
  return 0;
}
