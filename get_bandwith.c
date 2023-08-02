#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_TEMP_FILE_PATH_LENGTH 20

#define TMPF "/tmp/tx_packets3.tmp"
// #define PROC "/tmp/proc_net_dev"
#define PROC "/proc/net/dev"

char *get_temp_file_path() {
  static char temp_file_path[MAX_TEMP_FILE_PATH_LENGTH] = TMPF;
  return temp_file_path;
}

int is_device_name(const char *line) {
  // The line starts with "wl" or "eth", return true
  if (strncmp(line, "wl", 2) == 0 || strncmp(line, "eth", 3) == 0) {
    return 1;
  }
  return 0;
}

unsigned long get_tx_packets() {
  char line[MAX_LINE_LENGTH];

  unsigned long total_tx_packets = 0;
  FILE *fp;
  int line_count = 0;

  fp = fopen(PROC, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: could not open %s\n", PROC);
    return 0;
  }

  while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
    unsigned long tx_packets = 0;
    // Skip the first two lines
    if (line_count < 2) {
      line_count++;
      continue;
    }
    if (is_device_name(line)) {
      sscanf(line + strcspn(line, ":") + 1,
             "%*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %lu", &tx_packets);
      total_tx_packets += tx_packets;
    }
  }

  fclose(fp);

  // printf("total_tx_packets=%lu", total_tx_packets);
  return total_tx_packets;
}

// **************

unsigned long update_tx_packets(unsigned long tx_packets) {
  FILE *fp;
  unsigned long old_tx_packets = 0;

  char *temp_file_path = get_temp_file_path();

  fp = fopen(temp_file_path, "r+");
  if (fp == NULL) {
    fp = fopen(temp_file_path, "w+");
    if (fp == NULL) {
      fprintf(stderr, "Error: could not create temporary file\n");
      return 0;
    }
  } else {
    char line[MAX_LINE_LENGTH];
    fgets(line, MAX_LINE_LENGTH, fp);
    old_tx_packets = strtoul(line, NULL, 10);
  }

  unsigned long diff_tx_packets = tx_packets - old_tx_packets;

  rewind(fp);
  fprintf(fp, "%lu", tx_packets);
  fflush(fp);
  fclose(fp);

  if (1 < diff_tx_packets && diff_tx_packets < 21) {
    diff_tx_packets = 0;
  }
  return diff_tx_packets;
}

int check_ping_status() {
  FILE *fp;
  char value[10];

  fp = fopen("/tmp/ping.time", "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: cannot open file /tmp/ping.time\n");
    exit(1);
  }

  if (fgets(value, sizeof(value), fp) == NULL) {
    printf("Error: cannot read from file /tmp/ping.time\n");
    exit(1);
  }

  fclose(fp);

  if (strcmp(value, "1\n") == 0) {
    return 1;
  } else {
    return 0;
  }
}

int main() {
  unsigned long packets;

  packets = get_tx_packets();

  unsigned long diff_packet = update_tx_packets(packets);
  char s[5];
  snprintf(s, sizeof(s), "%4lu", diff_packet);
  // printf("%4s kb\n", s);

  //
  char state[15] = "Idle";

  int ping_status = check_ping_status();

  if (ping_status) {
    strcpy(state, "Good");
  }
  printf("{\"state\":\"%s\", \"text\": \"%s kb\"}", state, s);

  return 0;
}
