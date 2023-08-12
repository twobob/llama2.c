// save_to_file.c 

#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined _WIN32
    #include <io.h>   // for _access
    #include <fcntl.h> // For O_RDONLY
    #include "win.h"
    #include <direct.h>  // for creating directory
	#define OPEN_CMD _open
	#define CLOSE_CMD _close
	#define ACCESS_CMD _access
	#define MKDIR_CMD(path) _mkdir(path)
#else
    #include <unistd.h>
    #include <sys/mman.h>
	#include <sys/stat.h>
	#define OPEN_CMD open    
	#define CLOSE_CMD close
	#define ACCESS_CMD access
	#define MKDIR_CMD(path) mkdir(path, 0700)
#endif

#include "save_to_file.h" // Include header file

#include "base64_utils.h" // Include if base64_encode is defined here

void save_prompt_to_file(int saveFileBool, const char *prompt, const char *dirname, const char *timestamp, char *tokens_so_far) {
    if (saveFileBool > 0) {
        char decoded_prompt[MAX_STRING_SIZE];
        strncpy(decoded_prompt, base64_encode((const unsigned char *)prompt, 75), MAX_STRING_SIZE);

        char filename[MAX_FILENAME_LENGTH];
        snprintf(filename, MAX_FILENAME_LENGTH, "%s/%s_%s.txt", dirname, decoded_prompt, timestamp);

        FILE *output_file = fopen(filename, "w");
        if (!output_file) {
            fprintf(stderr, "Unable to open the %s file %s!\n", dirname, filename);
            return;
        }

        fprintf(output_file, "%s", tokens_so_far);
        memset(tokens_so_far, 0, sizeof(tokens_so_far));
        fclose(output_file);
    }
}

void finalize_log_file(int saveLogBool, FILE *timelog_file) {
    if (saveLogBool) {
        flush_log_buffer(timelog_file);
        fclose(timelog_file);
    }
}



long log_time_in_ms() {
    // return time in milliseconds, for benchmarking the model speed
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

#define LOG_BUFFER_SIZE 16
static long log_buffer[LOG_BUFFER_SIZE];
static size_t log_buffer_count = 0;

void handle_log_file(int saveLogBool, int groupLogBool, const char *dirname, const char *compiler_name, int checkpoint,  char *time_log) {
    if (saveLogBool) {
        FILE *timelog_file = NULL;
        if (!groupLogBool) {
            // individual log names per COMPILER
            snprintf(time_log, MAX_FILENAME_LENGTH, "%s/%s_%iM_timelog.csv", dirname, compiler_name, checkpoint);
        } else {
            // grouped into one file per compiler
            snprintf(time_log, MAX_FILENAME_LENGTH, "%s/All_%iM_timelog.csv", dirname, checkpoint);
        }

        timelog_file = fopen(time_log, "a");
        if (!timelog_file) {
            fprintf(stderr, "Unable to open the %s file %s!\\n", dirname, time_log);
            return;
        }
    
        // individual log headers per COMPILER
        // TODO EMBED ARGVs or some of them
        fprintf(timelog_file, "\\n%s-%i-%ld,", compiler_name, checkpoint, log_time_in_ms());
        
        fclose(timelog_file);
    }
}

int create_directory(const char *dirname) {
    if (ACCESS_CMD(dirname, 0) == -1) {
        fprintf(stderr, "Directory %s does not exist. Attempting to create it.\n", dirname);
        // If directory doesn't exist, create it
        if (MKDIR_CMD(dirname) == -1) {
            fprintf(stderr, "Unable to create directory %s!\n", dirname);
            return 1;
        }
    }
    return 0;
}


void initialize_file_saving(char *timestamp, char *tokens_so_far, FILE **output_file, FILE **timelog_file, char *time_log) {
    time_t t;
    struct tm *tm_info;

    t = time(NULL);
    tm_info = localtime(&t);
    strftime(timestamp, 26, "%Y%m%d%H%M%S", tm_info);

    tokens_so_far[0] = '\0';
    *output_file = NULL;
    *timelog_file = NULL;
}


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