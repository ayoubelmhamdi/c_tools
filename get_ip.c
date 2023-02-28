#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strncpy */
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#define true 1
#define false 0

void dev_ip(char name_wifi[], char s[]) {
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);

  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;

  /* I want IP address attached to "eth0","wlan0","wlp3s0" */
  strncpy(ifr.ifr_name, name_wifi, IFNAMSIZ - 1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);
  /* return result */
  strcpy(s, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void ip_from_dev(char s[]) {
  // TODO: improve
  // get the name of the wlan automatique
  char null_wifi[] = "0.0.0.0";
  dev_ip("eth0", s);
  if (strcmp(s, null_wifi) == 0) {
    dev_ip("wlp3s0", s);
  }
  if (strcmp(s, null_wifi) == 0) {
    dev_ip("wlan0", s);
  }
}

int check_internet_connection() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in server;
  struct hostent *host;
  host = gethostbyname("www.google.com");
  if (host == NULL) {
    // printf("Error resolving hostname\n");
    return 0;
  }
  server.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
  server.sin_family = AF_INET;
  server.sin_port = htons(80);

  int connected = connect(sock, (struct sockaddr *)&server, sizeof(server));
  close(sock);

  if (connected == 0) {
    return 1;
  } else {
    return 0;
  }
}

int main(void) {
  //
  char s[15];
  char state[15] = "Idle";

  int connection_status = check_internet_connection();

  if (connection_status) {
    strcpy(state, "Good");
  }

  ip_from_dev(s);
  printf("{\"state\":\"%s\", \"text\": \"%s\"}", state, s);

  return 0;
}
