#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include "win.h"
    #include <direct.h>
    #define access _access
    #define mkdir(path) _mkdir(path)
#else
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #define access _access
    #define mkdir(path) mkdir(path, 0700)
#endif

#include "save_to_file.h"
#include "base64_utils.h" 
#include "win.h"

#define LOG_BUFFER_SIZE 16
static long log_buffer[LOG_BUFFER_SIZE];
static size_t log_buffer_count = 0;

long log_time_in_ms() {
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return time.tv_sec * 1000 + time.tv_nsec / 1000000;
}



void initialize_file_saving(char *timestamp, char *tokens_so_far, FILE **output_file, FILE **timelog_file, char *time_log) {
    // for saving files
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(timestamp, 26, "%Y%m%d%H%M%S", tm_info);

    tokens_so_far[0] = '\0';

    *output_file = NULL; // Declare 
    *timelog_file = NULL; // Declare 
}

const char* MorK(const char *path) {
    // Find the last dot
    char *last_dot = strrchr(path, '.');

    // If the character before the last dot is 'M', 'm', 'K', or 'k', return it
    if (last_dot && (last_dot[-1] == 'M' || last_dot[-1] == 'm' || last_dot[-1] == 'K' || last_dot[-1] == 'k')) {
        static char result[2] = {0};  // static because we return a pointer to it
        result[0] = last_dot[-1];
        return result;
    }

    // Otherwise, return an empty string
    return "";
}


void handle_log_file(int saveLogBool, int groupLogBool, const char *dirname, const char *compiler_name, const char *mork,  int checkpoint, char *time_log, FILE **timelog_file) {
    if (saveLogBool && !*timelog_file) {
        if (!groupLogBool) {
            snprintf(time_log, MAX_FILENAME_LENGTH, "%s/%s_%i%s_timelog.csv", dirname, compiler_name, checkpoint,mork);
        } else {
            snprintf(time_log, MAX_FILENAME_LENGTH, "%s/All_%i%s_timelog.csv", dirname, checkpoint, mork);
        }

        *timelog_file = fopen(time_log, "a");
        if (!*timelog_file) {
            fprintf(stderr, "Unable to open the %s file %s!\n", dirname, time_log);
            return;
        }
		
		fprintf(stderr, "Opened the %s file %s\n", dirname, time_log);

        fprintf(*timelog_file, "\n%s-%i-%ld,", compiler_name, checkpoint, log_time_in_ms());
		fflush(*timelog_file);
		
		//printf("\n%s-%i-%ld,", compiler_name, checkpoint, log_time_in_ms());
		
		//printf("Header created, writing to file\n"); // Debug print
    }
}

void add_timestamp_to_log_buffer(long timestamp, FILE *timelog_file) {
    log_buffer[log_buffer_count] = timestamp;
    //printf("Added timestamp: %ld at index: %zu\n", timestamp, log_buffer_count); // Debug print
    log_buffer_count++;
    if (log_buffer_count == LOG_BUFFER_SIZE) {
        //printf("Buffer full, writing to file\n"); // Debug print
        for (size_t i = 0; i < LOG_BUFFER_SIZE; i++) {
            fprintf(timelog_file, "%ld,", log_buffer[i]);
		}
		fflush(timelog_file);
        log_buffer_count = 0;
    }
}

void flush_log_buffer(FILE *timelog_file) {
    for (size_t i = 0; i < log_buffer_count; i++) {
        fprintf(timelog_file, "%ld,", log_buffer[i]);
    }
	fflush(timelog_file);
    log_buffer_count = 0;
}

void finalize_log_file(int saveLogBool, FILE *timelog_file) {
    if (saveLogBool) {
        flush_log_buffer(timelog_file);
        fclose(timelog_file);
    }
}

void save_prompt_to_file(int saveFileBool, const char *prompt, const char *dirname, const char *timestamp, char *tokens_so_far) {
    if (saveFileBool > 0) {
        char decoded_prompt[MAX_STRING_SIZE];
        strncpy(decoded_prompt, base64_encode((const unsigned char *)prompt, 75), MAX_STRING_SIZE);

        char filename[MAX_FILENAME_LENGTH];
        snprintf(filename, MAX_FILENAME_LENGTH, "%s/%s_%s.txt", dirname, decoded_prompt, timestamp);

        FILE *output_file = fopen(filename, "w");
        if (!output_file) {
            fprintf(stderr, "Unable to open the %s file %s!\\n", dirname, filename);
            return;
        }
		

        fprintf(output_file, "%s", tokens_so_far);
		fflush(output_file);
        memset(tokens_so_far, 0, MAX_STRING_SIZE);
        fclose(output_file);
    }
}

int create_directory(const char *dirname) {
    if (access(dirname, 0) == -1) {
        fprintf(stderr, "Directory %s does not exist. Attempting to create it.\\n", dirname);
        if (mkdir(dirname) == -1) {
            fprintf(stderr, "Unable to create directory %s!\\n", dirname);
            return 1;
        }
    }
    return 0;
}

