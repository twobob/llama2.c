// save_to_file.c 

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <direct.h>

#include "save_to_file.h" // Include header file

#define LOG_BUFFER_SIZE 16
static long log_buffer[LOG_BUFFER_SIZE];
static size_t log_buffer_count = 0;


void add_timestamp_to_log_buffer(long timestamp, FILE *timelog_file) {
    log_buffer[log_buffer_count++] = timestamp;
    if (log_buffer_count == LOG_BUFFER_SIZE) {
        for (size_t i = 0; i < LOG_BUFFER_SIZE; i++) {
            fprintf(timelog_file, "%ld,", log_buffer[i]);
        }
        log_buffer_count = 0;
    }
}

void flush_log_buffer(FILE *timelog_file) {
    for (size_t i = 0; i < log_buffer_count; i++) {
        fprintf(timelog_file, "%ld,", log_buffer[i]);
    }
    log_buffer_count = 0;
}


void save_to_file(char *tokens_so_far, char *timestamp, char *dirname) {

  char filename[50];
  snprintf(filename, 50, "%s/%s.txt", dirname, timestamp);

  FILE *output_file = fopen(filename, "w");
  if (!output_file) {
    printf("Unable to open the inbox file %s!\n", filename);
    return;
  }

  fprintf(output_file, "%s", tokens_so_far);
   
  fclose(output_file);
}