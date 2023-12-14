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

#define CONNECTED 1
#define DISCONNECTED 0


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

int check_ping_status() {
    FILE *fp;
    char value[10];

    fp = fopen("/tmp/ping.time", "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open file /tmp/ping.time\n");
        return DISCONNECTED;
    }

    if (fgets(value, sizeof(value), fp) == NULL) {
        fprintf(stderr, "Error: cannot read from file /tmp/ping.time\n");
        fclose(fp);
        return DISCONNECTED;
    }

    fclose(fp);

    if (strcmp(value, "1\n") == 0)
        return CONNECTED;
    return DISCONNECTED;
}

int main(void) {
  //
  char s[15];
  char state[15] = "Idle";

  int connection_status = check_ping_status();

  if (connection_status) {
    strcpy(state, "Good");
  }

  ip_from_dev(s);
  printf("{\"state\":\"%s\", \"text\": \"%s\"}", state, s);

  return 0;
}
