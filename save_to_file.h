// save_to_file.h
#ifndef SAVE_TO_FILE_H
#define SAVE_TO_FILE_H

#pragma warning(disable: 4996)

#define open _open
#define close _close

#include <stdio.h>

#define MAX_FILENAME_LENGTH 127
#define MAX_STRING_SIZE 5000


const char* MorK(const char *path) ; 

int create_directory(const char *dirname);

long log_time_in_ms();

void handle_log_file(int saveLogBool, int groupLogBool, const char *dirname, const char *compiler_name, const char *mork,  int checkpoint, char *time_log, FILE **timelog_file) ;


void initialize_file_saving(char *timestamp, char *tokens_so_far, FILE **output_file, FILE **timelog_file, char *time_log) ;

void save_prompt_to_file(int saveFileBool, const char *prompt, const char *dirname, const char *timestamp, char *tokens_so_far);

void finalize_log_file(int saveLogBool, FILE *timelog_file);


void add_timestamp_to_log_buffer(long timestamp, FILE *timelog_file);
void flush_log_buffer(FILE *timelog_file);

void save_to_file(char *tokens_so_far, char *timestamp, char *dirname);

#endif