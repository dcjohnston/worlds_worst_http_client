#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAX_BUFF_SIZE 5096

void send_http_request(int sockfd) {
  char send_buff[MAX_BUFF_SIZE];
  char *version_header = "HTTP/1.1\r\n";
  char *method_and_path = "GET /\r\n";
  int bytes_sent;
  strcpy(send_buff, version_header);
  strcpy(send_buff, method_and_path);
  bytes_sent = send(sockfd, send_buff, strlen(send_buff), 0);
  if (bytes_sent != strlen(send_buff)) {
    fprintf(stderr, "An error has occured while sending the HTTP request.");
    exit(1);
  } else {
    printf("We have successfully sent the HTTP Request\n");
  }
}

void receive_http_response(int sockfd) {
  char recv_buff[MAX_BUFF_SIZE];
  int bytes_received;
  bytes_received = recv(sockfd, recv_buff, MAX_BUFF_SIZE, 0);
  printf("Received %d bytes\n", bytes_received);
  printf("The response follows:\n");
  printf("%s\n", recv_buff);
}

int prepare_connection(char *host) {
  int status;
  struct in_addr *addr;
  struct addrinfo *hints = malloc(sizeof(struct addrinfo)); //should actually use stack allocation instead of heap (declare as struct)
  struct addrinfo *target_list, *p;
  char *ipver = "IPv4";
  char ipstr[INET6_ADDRSTRLEN];
  int sock;
  memset(hints, 0, sizeof(struct addrinfo));
  hints->ai_family = AF_INET; //IPV4
  hints->ai_socktype = SOCK_STREAM; // TCP
  hints->ai_flags = AI_PASSIVE;
  if ((status = getaddrinfo(host, "http", hints, &target_list)) != 0) {
    fprintf(stderr, "An error has occured while fetching host info %s\n", gai_strerror(status));
    exit(1);
  }
  for (p = target_list; p != NULL; p = p->ai_next) {
    struct sockaddr_in *target = (struct sockaddr_in *)p->ai_addr;
    addr = &(target->sin_addr);
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf("Targetting host %s @ %s: %s\n", host, ipver, ipstr);
    sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol); //create socket of proper type, protocol, etc
    if ((status = connect(sock, p->ai_addr, p->ai_addrlen) != 0)) {
      fprintf(stderr, "An error has occured while connecting.\n");
    } else {
      printf("We have successfully connected!\n");
      break;
    }
  }
  free(hints);
  freeaddrinfo(target_list);
  return sock;
}

int main(int argc, char** argv){
  if (argc != 3) {
    printf("Usage: client <host> <path> <method>\n");
    exit(1);
  }
  int sock = prepare_connection(argv[1]);
  send_http_request(sock);
  receive_http_response(sock);
  close(sock);
  return 0;
}
