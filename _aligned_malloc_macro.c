#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>  // <- Including it here so _stat struct is defined for both platforms

// Cross-platform Macros & Includes
#if defined(_WIN32)
    #include <io.h>
    #include <fcntl.h>
    #include <direct.h>
    #include <windows.h>
	//winnt.h:5917: #define FILE_OPEN 0x00000001 . so avoid
    #define _FILE_OPEN(path, mode) _open(path, mode)
    #define _FILE_CLOSE(fd) _close(fd)
    #define _FILE_STAT(fd, statStruct) _fstat(fd, statStruct)
    #define ALIGNED_ALLOC(alignment, size) _aligned_malloc(size, alignment)
    #define ALIGNED_FREE(ptr) _aligned_free(ptr)
    typedef struct _stat FileStat;
#else
    #include <unistd.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #define _FILE_OPEN(path, mode) open(path, mode)
    #define _FILE_CLOSE(fd) close(fd)
    #define _FILE_STAT(fd, statStruct) fstat(fd, statStruct)
    #define ALIGNED_ALLOC(alignment, size) aligned_alloc(alignment, size)
    #define ALIGNED_FREE(ptr) free(ptr)
    typedef struct stat FileStat;
#endif

#define ALIGN_CHECK(ptr, alignment) (((uintptr_t)(ptr)) % (alignment) == 0)

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return 1;
    }

    int successCount = 0;
    int failCount = 0;

#if defined(_WIN32)
    char* directory_path = argv[1];
    char searchPath[512];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", directory_path);
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No files found\n");
        return 2;
    } 

    do {
        const char *filename = findFileData.cFileName;

        if (filename[1] != '.' && filename[0] != '.') {
            char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "%s\\%s", directory_path, filename);

            int fd = _FILE_OPEN(fullPath, O_RDONLY);
            if (fd == -1) {
                perror("Error opening file");
                continue;
            }

            FileStat fileStat;
            _FILE_STAT(fd, &fileStat);
            int64_t fileSize = (int64_t)fileStat.st_size;

            float* data = (float*) ALIGNED_ALLOC(1024, fileSize);
            if (!data) {
                fprintf(stderr, "Failed to allocate aligned memory for %s\n", filename);
                failCount++;
                _FILE_CLOSE(fd);
                continue;
            }

            if (ALIGN_CHECK(data, 1024)) {
                printf("Memory for %s is 1024-byte aligned: %p\n", filename, (void*)data);
                successCount++;
            } else {
                printf("Memory for %s is NOT 1024-byte aligned.\n", filename);
                failCount++;
            }
            printf("Attempting to free: %s data which is %lld\n", fullPath,  fileSize);
            ALIGNED_FREE(data);
            _FILE_CLOSE(fd);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    DIR *dir = opendir(argv[1]);
    if (!dir) {
        perror("Failed to open directory");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            const char *filename = entry->d_name;

            if (filename[0] != '.') {
                char fullPath[512];
                snprintf(fullPath, sizeof(fullPath), "%s/%s", argv[1], filename);


                printf("Attempting to open: %s\n", fullPath);

                int fd = _FILE_OPEN(fullPath, O_RDONLY);
                if (fd == -1) {
                    perror("Error opening file");
                    continue;
                }

                FileStat fileStat;
                _FILE_STAT(fd, &fileStat);
                int64_t fileSize = (int64_t)fileStat.st_size;

                float* data = (float*) ALIGNED_ALLOC(1024, fileSize);
                if (!data) {
                    fprintf(stderr, "Failed to allocate aligned memory for %s\n", filename);
                    failCount++;
                    _FILE_CLOSE(fd);
                    continue;
                }

                if (ALIGN_CHECK(data, 1024)) {
                    printf("Memory for %s is 1024-byte aligned: %p\n", filename, (void*)data);
                    successCount++;
                } else {
                    printf("Memory for %s is NOT 1024-byte aligned.\n", filename);
                    failCount++;
                }
                printf("Attempting to free: %s data which is %p\n", fullPath,  (void*)data);
                ALIGNED_FREE(data);
                _FILE_CLOSE(fd);
            }
        }
    }
    closedir(dir);
#endif

    printf("\n\nSummary:\n");
    printf("Successful Operations: %d\n", successCount);
    printf("Failed Operations: %d\n", failCount);

    return 0;
}
