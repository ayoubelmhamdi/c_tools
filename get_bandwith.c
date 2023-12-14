#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 256
#define MAX_TEMP_FILE_PATH_LENGTH 20

#define TMPF "/tmp/tx_packets3.tmp"
// #define PROC "/tmp/proc_net_dev"
#define PROC "/proc/net/dev"

#define CONNECTED 1
#define DISCONNECTED 0


int check_ping_status();
char *get_temp_file_path();
int is_device_name(char *line);
unsigned long get_tx_packets();
void write_string_to_file(const char *str);

char *ltrim(char *s) {
    while (isspace(*s))
        s++;
    return s;
}

char *get_temp_file_path() {
    static char temp_file_path[MAX_TEMP_FILE_PATH_LENGTH] = TMPF;
    return temp_file_path;
}

int is_device_name(char *line) {
    // The line starts with "wl" or "eth", return true
    if (strncmp(ltrim(line), "wl", 2) == 0 || strncmp(line, "eth", 3) || strncmp(line, "usb", 3) == 0) return 1;
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

    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned long tx_packets = 0;
        unsigned long rx_packets = 0;
        // Skip the first two lines
        if (line_count < 2) {
            line_count++;
            continue;
        }
        if (is_device_name(line)) {
            if (sscanf(line, "%*s %lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %lu", &rx_packets, &tx_packets) == 2) {
                total_tx_packets += rx_packets;
                // total_tx_packets += tx_packets + rx_packets;
            }
        }
    }
    fclose(fp);
    return total_tx_packets;
}

int check_ping_status() {
    FILE *fp;
    char value[10];

    // TODO: check if ping-status pid exists
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

    if (strncmp(value, "1", 1) == 0)    return CONNECTED;
    else                                return DISCONNECTED;
}

void write_string_to_file(const char *str) {
    FILE *file = fopen(TMPF, "w+");
    if (file != NULL) {
        fputs(str, file);
        fclose(file);
    } else {
        printf("Error opening file!\n");
    }
}

int main() {
    char state[15] = "Idle";
    int ping_status = 0;
    unsigned long delta = 0, new =0, old = 0;
    char output[200];

    while (1) {
        new = get_tx_packets();
        ping_status = check_ping_status();

        delta = (new - old)/1024;

        if (ping_status) {
            if (delta > 5) sprintf(output, "%lu kb  |\n", delta);
            else sprintf(output, " |\n");
        } else sprintf(output, "  |\n");

        write_string_to_file(output);

        old = new;
        sleep(1);
    }

    // i3status-rs
    // printf("{\"state\":\"%s\", \"text\": \"%s kb\"}", state, bandwith);
    return 0;
}
