// save_to_file.h
#ifndef SAVE_TO_FILE_H
#define SAVE_TO_FILE_H

#include <stdio.h>

#define MAX_FILENAME_LENGTH 127
#define MAX_STRING_SIZE 5000

void add_timestamp_to_log_buffer(long timestamp, FILE *timelog_file);
void flush_log_buffer(FILE *timelog_file);

void save_to_file(char *tokens_so_far, char *timestamp, char *dirname);

#endif