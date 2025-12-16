#ifndef PLAT_H
#define PLAT_H
#define _CRT_SECURE_NO_WARNINGS
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#include <windows.h>
#define PLATFORM_SLEEP(ms) Sleep(ms)
#define PATH_SEP '\\'
typedef HMODULE PlatformLibraryHandle;
typedef CRITICAL_SECTION PlatformMutex; /* Simple fast mutex */
#else
#include <dirent.h>
#include <dlfcn.h>
#include <limits.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>
#define PLATFORM_SLEEP(ms) usleep((ms) * 1000)
#define PATH_SEP '/'
typedef void *PlatformLibraryHandle;
typedef pthread_mutex_t PlatformMutex;
#endif
#define MAX_PATH_CUSTOM 512

typedef struct {
#ifdef _WIN32
  HANDLE hFind;
  WIN32_FIND_DATA findData;
  int firstRead;
#else
  DIR *dp;
#endif
  char path[MAX_PATH_CUSTOM];
} PlatformDirHandle;

#if 1

int platform_init(void);

/* --- Core Abstractions & Declarations --- */
int platform_path_join(const char *dir, const char *file, char *buffer,
                       int size);
PlatformDirHandle *platform_dir_open(const char *path);
int platform_dir_read(PlatformDirHandle *dirHandle, char *filenameBuffer,
                      int bufferSize);
void platform_dir_close(PlatformDirHandle *dirHandle);
int platform_file_exists(const char *path);
int platform_is_dir(const char *path);
long platform_file_size(const char *path);
int platform_file_delete(const char *path);
int platform_file_rename(const char *oldPath, const char *newPath);
int platform_file_copy(const char *srcPath, const char *dstPath);
void platform_normalize_path(char *path);
int platform_create_dir(const char *path);
int platform_create_dir_recursive(const char *path);
long long platform_get_timestamp_ms(void);
const char *platform_get_env(const char *name);
int platform_get_executable_path(char *buffer, int size);
int platform_get_executable_dir(char *buffer, int size);
int platform_get_current_working_dir(char *buffer, int size);
int platform_get_user_home_dir(char *buffer, int size);
void platform_delay_ms(int ms);
int platform_get_full_path(const char *relativePath, char *buffer, int size);
int platform_get_temp_path(char *buffer, int size);
int platform_get_temp_filename(char *buffer, int size, const char *prefix);
unsigned int platform_get_process_id(void);
unsigned int platform_get_thread_id(void);
long long platform_get_performance_counter(void);
long long platform_get_performance_frequency(void);
double platform_get_time_s(void);
int platform_get_num_cpu_cores(void);
PlatformLibraryHandle platform_library_load(const char *filename);
void *platform_library_get_func(PlatformLibraryHandle handle,
                                const char *funcName);
int platform_library_unload(PlatformLibraryHandle handle);
const char *platform_library_get_error(void);
int platform_mutex_init(PlatformMutex *mutex);
int platform_mutex_lock(PlatformMutex *mutex);
int platform_mutex_unlock(PlatformMutex *mutex);
int platform_mutex_destroy(PlatformMutex *mutex);

/* --- Memory Safety (Wrappers) --- */
/* NOTE: Using the new platform_safe_malloc/free wrappers requires updating the
 * application code */
#define platform_malloc(size) platform_safe_malloc(size, __FILE__, __LINE__)
#define platform_free(ptr) platform_safe_free(ptr, __FILE__, __LINE__)
void *platform_safe_malloc(size_t size, const char *file, int line);
void platform_safe_free(void *ptr, const char *file, int line);
#endif

#ifdef PLATFORM_IMPLEMENTATION

/* --- Implementation (Safe Memory) --- */
void *platform_safe_malloc(size_t size, const char *file, int line) {
  void *ptr = malloc(size);
  if (ptr == NULL && size > 0) {
    fprintf(stderr,
            "FATAL ERROR: Memory allocation failed for %lu bytes in %s at line "
            "%d\n",
            (unsigned long)size, file, line);
    exit(EXIT_FAILURE);
  }
  return ptr;
}
void platform_safe_free(void *ptr, const char *file, int line) {
  if (ptr != NULL) {
    free(ptr);
  }
}

#ifdef _WIN32

/* --- Dynamic Libraries --- */
PlatformLibraryHandle platform_library_load(const char *filename) {
  return LoadLibrary(filename);
}
void *platform_library_get_func(PlatformLibraryHandle handle,
                                const char *funcName) {
  return (void *)GetProcAddress(handle, funcName);
}
int platform_library_unload(PlatformLibraryHandle handle) {
  return FreeLibrary(handle);
}
const char *platform_library_get_error(void) {
  /* Windows error messages are complex; for a simple plat.h, we return a static
   * string. */
  return "Windows dynamic library error occurred (check system logs).";
}

/* --- Synchronization --- */
int platform_mutex_init(PlatformMutex *mutex) {
  InitializeCriticalSection(mutex);
  return 1;
}
int platform_mutex_lock(PlatformMutex *mutex) {
  EnterCriticalSection(mutex);
  return 1;
}
int platform_mutex_unlock(PlatformMutex *mutex) {
  LeaveCriticalSection(mutex);
  return 1;
}
int platform_mutex_destroy(PlatformMutex *mutex) {
  DeleteCriticalSection(mutex);
  return 1;
}

/* --- System/Timing/Process --- */
unsigned int platform_get_process_id(void) {
  return (unsigned int)GetCurrentProcessId();
}
unsigned int platform_get_thread_id(void) {
  return (unsigned int)GetCurrentThreadId();
}
long long platform_get_performance_counter(void) {
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return counter.QuadPart;
}
long long platform_get_performance_frequency(void) {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  return frequency.QuadPart;
}
double platform_get_time_s(void) {
  static long long frequency = 0;
  if (frequency == 0) {
    frequency = platform_get_performance_frequency();
  }
  return (double)platform_get_performance_counter() / (double)frequency;
}
int platform_get_num_cpu_cores(void) {
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return (int)sysInfo.dwNumberOfProcessors;
}

/* --- Path/File/Dir --- */
int platform_file_rename(const char *oldPath, const char *newPath) {
  return MoveFile(oldPath, newPath);
}
int platform_file_copy(const char *srcPath, const char *dstPath) {
  return CopyFile(srcPath, dstPath, FALSE); /* Overwrite if exists */
}
int platform_get_temp_path(char *buffer, int size) {
  return GetTempPath(size, buffer);
}
int platform_get_temp_filename(char *buffer, int size, const char *prefix) {
  char tempDir[MAX_PATH_CUSTOM];
  if (platform_get_temp_path(tempDir, MAX_PATH_CUSTOM) == 0)
    return 0;
  return GetTempFileName(tempDir, prefix, 0, buffer) > 0;
}
int platform_get_executable_dir(char *buffer, int size) {
  if (!platform_get_executable_path(buffer, size))
    return 0;
  char *last_sep = strrchr(buffer, PATH_SEP);
  if (last_sep) {
    *last_sep = '\0';
    return 1;
  }
  return 0;
}
int platform_get_current_working_dir(char *buffer, int size) {
  return GetCurrentDirectory(size, buffer);
}
int platform_get_user_home_dir(char *buffer, int size) {
  const char *home = platform_get_env("USERPROFILE");
  if (home) {
    strncpy(buffer, home, size);
    buffer[size - 1] = '\0';
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
void platform_normalize_path(char *path) {
  char *p;
  for (p = path; *p; p++) {
    if (*p == '/') {
      *p = PATH_SEP;
    }
  }
}
int platform_create_dir(const char *path) {
  if (platform_is_dir(path)) {
    return 1;
  }
  return CreateDirectory(path, NULL);
}
/* Recursive directory creation (Implemented) */
int platform_create_dir_recursive(const char *path) {
  char tempPath[MAX_PATH_CUSTOM];
  char *p;
  int success = 1;

  if (strlen(path) >= MAX_PATH_CUSTOM)
    return 0;
  strcpy(tempPath, path);
  platform_normalize_path(tempPath);

  for (p = tempPath + 1; *p; p++) {
    if (*p == PATH_SEP) {
      *p = '\0';
      if (!platform_is_dir(tempPath)) {
        if (!platform_create_dir(tempPath)) {
          success = 0;
          break;
        }
      }
      *p = PATH_SEP;
    }
  }
  if (success && !platform_is_dir(tempPath)) {
    success = platform_create_dir(tempPath);
  }
  return success;
}

/* Existing Core Functions */
long long platform_get_timestamp_ms(void) {
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  ULONGLONG windowsTime =
      ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
  windowsTime -= 116444736000000000ULL;
  return (long long)(windowsTime / 10000);
}
const char *platform_get_env(const char *name) { return getenv(name); }
int platform_get_executable_path(char *buffer, int size) {
  DWORD result = GetModuleFileName(NULL, buffer, size);
  if (result > 0 && result < size) {
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
void platform_delay_ms(int ms) { PLATFORM_SLEEP(ms); }
int platform_get_full_path(const char *relativePath, char *buffer, int size) {
  if (GetFullPathName(relativePath, size, buffer, NULL) > 0) {
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
int platform_path_join(const char *dir, const char *file, char *buffer,
                       int size) {
  int dirLen;
  dirLen = (int)strlen(dir);
  if (dirLen + (int)strlen(file) + 2 > size) {
    return 0;
  }
  strcpy(buffer, dir);
  if (dirLen > 0 && buffer[dirLen - 1] != PATH_SEP) {
    buffer[dirLen] = PATH_SEP;
    dirLen++;
  }
  strcpy(buffer + dirLen, file);
  return 1;
}
PlatformDirHandle *platform_dir_open(const char *path) {
  PlatformDirHandle *dirHandle;
  char searchPath[MAX_PATH_CUSTOM];
  dirHandle = (PlatformDirHandle *)platform_malloc(sizeof(PlatformDirHandle));
  if (!dirHandle) {
    return (PlatformDirHandle *)NULL;
  }
  dirHandle->firstRead = 1;
  strncpy(dirHandle->path, path, MAX_PATH_CUSTOM);
  dirHandle->path[MAX_PATH_CUSTOM - 1] = '\0';
  platform_path_join(path, "*", searchPath, MAX_PATH_CUSTOM);
  dirHandle->hFind = FindFirstFile(searchPath, &dirHandle->findData);
  if (dirHandle->hFind == INVALID_HANDLE_VALUE) {
    platform_free(dirHandle);
    return (PlatformDirHandle *)NULL;
  }
  return dirHandle;
}
int platform_dir_read(PlatformDirHandle *dirHandle, char *filenameBuffer,
                      int bufferSize) {
  if (dirHandle->firstRead) {
    dirHandle->firstRead = 0;
  } else {
    if (!FindNextFile(dirHandle->hFind, &dirHandle->findData)) {
      return 0;
    }
  }
  strncpy(filenameBuffer, dirHandle->findData.cFileName, bufferSize);
  filenameBuffer[bufferSize - 1] = '\0';
  return 1;
}
void platform_dir_close(PlatformDirHandle *dirHandle) {
  if (dirHandle->hFind != INVALID_HANDLE_VALUE) {
    FindClose(dirHandle->hFind);
  }
  platform_free(dirHandle);
}
int platform_file_exists(const char *path) {
  DWORD dwAttrib = GetFileAttributes(path);
  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
          !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
int platform_is_dir(const char *path) {
  DWORD dwAttrib = GetFileAttributes(path);
  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
          (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
long platform_file_size(const char *path) {
  HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  DWORD sizeHigh, sizeLow;
  long size;
  if (hFile == INVALID_HANDLE_VALUE) {
    return -1;
  }
  sizeLow = GetFileSize(hFile, &sizeHigh);
  size = (long)(((LONGLONG)sizeHigh << 32) | sizeLow);
  CloseHandle(hFile);
  return size;
}
int platform_file_delete(const char *path) { return DeleteFile(path); }

#else /* POSIX (Linux, macOS, etc.) */

/* --- Dynamic Libraries --- */
PlatformLibraryHandle platform_library_load(const char *filename) {
  return dlopen(filename, RTLD_LAZY);
}
void *platform_library_get_func(PlatformLibraryHandle handle,
                                const char *funcName) {
  return dlsym(handle, funcName);
}
int platform_library_unload(PlatformLibraryHandle handle) {
  return dlclose(handle) == 0;
}
const char *platform_library_get_error(void) { return dlerror(); }

/* --- Synchronization --- */
int platform_mutex_init(PlatformMutex *mutex) {
  return pthread_mutex_init(mutex, NULL) == 0;
}
int platform_mutex_lock(PlatformMutex *mutex) {
  return pthread_mutex_lock(mutex) == 0;
}
int platform_mutex_unlock(PlatformMutex *mutex) {
  return pthread_mutex_unlock(mutex) == 0;
}
int platform_mutex_destroy(PlatformMutex *mutex) {
  return pthread_mutex_destroy(mutex) == 0;
}

/* --- System/Timing/Process --- */
unsigned int platform_get_process_id(void) { return (unsigned int)getpid(); }
unsigned int platform_get_thread_id(void) {
/* Use syscall to get the unique thread ID on Linux. */
#ifdef __linux__
  return (unsigned int)syscall(SYS_gettid);
#elif defined(__APPLE__)
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  return (unsigned int)tid;
#else
  /* Fallback for other POSIX, less reliable for threads */
  return platform_get_process_id();
#endif
}
long long platform_get_performance_counter(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
long long platform_get_performance_frequency(void) { return 1000000000LL; }
double platform_get_time_s(void) {
  return (double)platform_get_performance_counter() /
         (double)platform_get_performance_frequency();
}
int platform_get_num_cpu_cores(void) {
  return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

/* --- Path/File/Dir --- */
int platform_file_rename(const char *oldPath, const char *newPath) {
  return rename(oldPath, newPath) == 0;
}
int platform_file_copy(const char *srcPath, const char *dstPath) {
  /* POSIX does not have a simple copy function. We rely on standard copy
   * utility. */
  int c;
  FILE *src = fopen(srcPath, "rb");
  FILE *dst = fopen(dstPath, "wb");
  if (!src || !dst) {
    if (src)
      fclose(src);
    if (dst)
      fclose(dst);
    return 0;
  }
  while ((c = fgetc(src)) != EOF) {
    fputc(c, dst);
  }
  fclose(src);
  fclose(dst);
  return 1;
}
int platform_get_temp_path(char *buffer, int size) {
  const char *tempDir = platform_get_env("TMPDIR");
  if (!tempDir)
    tempDir = platform_get_env("TEMP");
  if (!tempDir)
    tempDir = platform_get_env("TMP");
  if (!tempDir)
    tempDir = "/tmp";

  if (strncpy(buffer, tempDir, size) != NULL) {
    buffer[size - 1] = '\0';
    platform_normalize_path(buffer);
    if (buffer[strlen(buffer) - 1] != PATH_SEP) {
      if ((int)strlen(buffer) + 1 < size) {
        strcat(buffer, "/");
      }
    }
    return (int)strlen(buffer);
  }
  return 0;
}
int platform_get_temp_filename(char *buffer, int size, const char *prefix) {
  char template[MAX_PATH_CUSTOM];

  if (platform_get_temp_path(template, MAX_PATH_CUSTOM) == 0)
    return 0;

  if (snprintf(template + strlen(template), MAX_PATH_CUSTOM - strlen(template),
               "%sXXXXXX", prefix) >= MAX_PATH_CUSTOM - strlen(template) ||
      snprintf(buffer, size, "%s", template) >= size) {
    return 0;
  }

  int fd = mkstemp(buffer);
  if (fd != -1) {
    close(fd);
    platform_file_delete(buffer);
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
int platform_get_executable_dir(char *buffer, int size) {
  if (!platform_get_executable_path(buffer, size))
    return 0;
  char *last_sep = strrchr(buffer, PATH_SEP);
  if (last_sep) {
    *last_sep = '\0';
    return 1;
  }
  return 0;
}
int platform_get_current_working_dir(char *buffer, int size) {
  return getcwd(buffer, size) != NULL;
}
int platform_get_user_home_dir(char *buffer, int size) {
  const char *home = platform_get_env("HOME");
  if (home) {
    strncpy(buffer, home, size);
    buffer[size - 1] = '\0';
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
void platform_normalize_path(char *path) {
  char *p;
  for (p = path; *p; p++) {
    if (*p == '\\') {
      *p = PATH_SEP;
    }
  }
}
int platform_create_dir(const char *path) {
  if (platform_is_dir(path)) {
    return 1;
  }
  return mkdir(path, 0755) == 0;
}
/* Recursive directory creation (Implemented) */
int platform_create_dir_recursive(const char *path) {
  char tempPath[MAX_PATH_CUSTOM];
  char *p;
  int success = 1;

  if (strlen(path) >= MAX_PATH_CUSTOM)
    return 0;
  strcpy(tempPath, path);
  platform_normalize_path(tempPath);

  /* Start after the root separator to handle /home/user or C:\ */
  for (p = tempPath + 1; *p; p++) {
    if (*p == PATH_SEP) {
      *p = '\0';
      if (!platform_is_dir(tempPath)) {
        if (!platform_create_dir(tempPath)) {
          success = 0;
          break;
        }
      }
      *p = PATH_SEP;
    }
  }
  if (success && !platform_is_dir(tempPath)) {
    success = platform_create_dir(tempPath);
  }
  return success;
}

/* Existing Core Functions */
long long platform_get_timestamp_ms(void) {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) != 0) {
    return 0;
  }
  return (long long)tv.tv_sec * 1000 + (long long)tv.tv_usec / 1000;
}
const char *platform_get_env(const char *name) { return getenv(name); }
int platform_get_executable_path(char *buffer, int size) {
  ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
  if (len != -1) {
    buffer[len] = '\0';
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
void platform_delay_ms(int ms) { PLATFORM_SLEEP(ms); }
int platform_get_full_path(const char *relativePath, char *buffer, int size) {
  if (realpath(relativePath, buffer) != NULL) {
    platform_normalize_path(buffer);
    return 1;
  }
  return 0;
}
int platform_path_join(const char *dir, const char *file, char *buffer,
                       int size) {
  int dirLen;
  dirLen = (int)strlen(dir);
  if (dirLen + (int)strlen(file) + 2 > size) {
    return 0;
  }
  strcpy(buffer, dir);
  if (dirLen > 0 && buffer[dirLen - 1] != PATH_SEP) {
    buffer[dirLen] = PATH_SEP;
    dirLen++;
  }
  strcpy(buffer + dirLen, file);
  return 1;
}
PlatformDirHandle *platform_dir_open(const char *path) {
  PlatformDirHandle *dirHandle;
  dirHandle = (PlatformDirHandle *)platform_malloc(sizeof(PlatformDirHandle));
  if (!dirHandle) {
    return (PlatformDirHandle *)NULL;
  }
  dirHandle->dp = opendir(path);
  if (!dirHandle->dp) {
    platform_free(dirHandle);
    return (PlatformDirHandle *)NULL;
  }
  return dirHandle;
}
int platform_dir_read(PlatformDirHandle *dirHandle, char *filenameBuffer,
                      int bufferSize) {
  struct dirent *ep;
  ep = readdir(dirHandle->dp);
  if (ep) {
    strncpy(filenameBuffer, ep->d_name, bufferSize);
    filenameBuffer[bufferSize - 1] = '\0';
    return 1;
  }
  return 0;
}
void platform_dir_close(PlatformDirHandle *dirHandle) {
  if (dirHandle->dp) {
    closedir(dirHandle->dp);
  }
  platform_free(dirHandle);
}
int platform_file_exists(const char *path) {
  struct stat buffer;
  return stat(path, &buffer) == 0 && !platform_is_dir(path);
}
int platform_is_dir(const char *path) {
  struct stat buffer;
  return stat(path, &buffer) == 0 && S_ISDIR(buffer.st_mode);
}
long platform_file_size(const char *path) {
  struct stat buffer;
  if (stat(path, &buffer) != 0) {
    return -1;
  }
  return (long)buffer.st_size;
}
int platform_file_delete(const char *path) { return unlink(path) == 0; }

#endif /* _WIN32 / POSIX */

#define MAX_FILENAME 256

char *platform_find_next_filename(const char *baseFilename, const char *ext,
                                  char *buffer) {
  char testFilename[MAX_FILENAME];
  int version = 0;
  size_t baseLen;
  int i;
  char baseNoNum[MAX_FILENAME];
  int numDigits = 0;

  sprintf(testFilename, "%s.%s", baseFilename, ext);
  if (!platform_file_exists(testFilename)) {
    strcpy(buffer, testFilename);
    return buffer;
  }

  baseLen = strlen(baseFilename);
  for (i = (int)baseLen; i > 0; i--) {
    if (baseFilename[i - 1] >= '0' && baseFilename[i - 1] <= '9') {
      numDigits++;
    } else {
      break;
    }
  }

  if (numDigits > 0) {
    strncpy(baseNoNum, baseFilename, baseLen - numDigits);
    baseNoNum[baseLen - numDigits] = '\0';
    version = atoi(baseFilename + (baseLen - numDigits));
    version++;
  } else {
    strncpy(baseNoNum, baseFilename, MAX_FILENAME);
    baseNoNum[MAX_FILENAME - 1] = '\0';
    version = 1;
  }

  while (1) {
    sprintf(buffer, "%s%d.%s", baseNoNum, version, ext);
    if (!platform_file_exists(buffer)) {
      return buffer;
    }
    version++;
  }
}

int platform_init(void) { return 0; }

#endif /* PLATOFRM_IMPLEMENTATION */

#endif /* PLAT_H */
