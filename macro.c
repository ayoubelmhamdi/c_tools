#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_TEMP_FILE_PATH_LENGTH 20

#define TMPF "/tmp/tx_packets3.tmp"
// #define PROC "/tmp/proc_net_dev"
#define PROC "/proc/net/dev"
#define PING_FILE "/tmp/ping.time"

#define CONNECTED 1
#define DISCONNECTED 0

char *get_temp_file_path() {
  static char temp_file_path[MAX_TEMP_FILE_PATH_LENGTH] = TMPF;
  return temp_file_path;
}

int is_device_name(const char *line) {
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
  return total_tx_packets;
}

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

  unsigned long diff_tx_packets = 0;
  if (tx_packets - old_tx_packets > 21) {
    diff_tx_packets = tx_packets - old_tx_packets;
  }

  rewind(fp);
  fprintf(fp, "%lu", tx_packets);
  fflush(fp);
  fclose(fp);

  if (1 < diff_tx_packets && diff_tx_packets < 21) {
    diff_tx_packets = 0;
  }

  return diff_tx_packets;
}

#define openfile(fp, s)                                                        \
  ({                                                                           \
    int isopened = 1;                                                          \
    fp = fopen(s, "r");                                                        \
    if (fp == NULL) {                                                          \
      fprintf(stderr, "Error: cannot open file %s\n", s);                      \
      isopened = 0;                                                            \
    }                                                                          \
    isopened;                                                                  \
  })

#define readLineFromFile(fp, s)                                                \
  ({                                                                           \
    int isreaded = 1;                                                          \
    if (fgets(value, sizeof(value), fp) == NULL) {                             \
      fprintf(stderr, "Error: cannot read from file %s\n", s);                 \
      fclose(fp);                                                              \
      isreaded = 0;                                                            \
    }                                                                          \
    isreaded;                                                                  \
  })

#define num_packets()                                                          \
  ({                                                                           \
    char bandwith[5];                                                          \
    unsigned long packets;                                                     \
    unsigned long diff_packet;                                                 \
                                                                               \
    packets = get_tx_packets();                                                \
    diff_packet = update_tx_packets(packets);                                  \
    snprintf(bandwith, sizeof(bandwith), "%4lu", diff_packet);                 \
                                                                               \
    bandwith;                                                                  \
  })

#define isHasEqualOne(value)                                                   \
  ({                                                                           \
    int is_equal_one = 0;                                                      \
    if (strcmp(value, "1\n") == 0)                                             \
      is_equal_one = 1;                                                        \
    is_equal_one;                                                              \
  })

int check_ping_status() {
  FILE *fp;
  char value[10];
  if (openfile(fp, PING_FILE) && readLineFromFile(fp, PING_FILE))
    if (isHasEqualOne(value))
      return CONNECTED;
  return DISCONNECTED;
}

#define ping_status()                                                          \
  ({                                                                           \
    char state[15] = "Idle";                                                   \
    if (check_ping_status()) {                                                 \
      strcpy(state, "Good");                                                   \
    }                                                                          \
    state;                                                                     \
  })

int main() {

  char *state = ping_status();
  char *bandwith = num_packets();

  printf("{\"state\":\"%s\", \"text\": \"%s kb\"}", state, bandwith);

  return 0;
}
