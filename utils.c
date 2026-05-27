#include "login.h"

char dataDir[MAX_PATH_LEN];

void getAppRelativePath(void) {
    char exePath[MAX_PATH_LEN];
    GetModuleFileNameA(NULL, exePath, MAX_PATH_LEN);
    char *pos = strrchr(exePath, '\\');
    if (pos) {
        *pos = '\0';
        sprintf(dataDir, "%s\\data", exePath);
    }
}

void ensure_dir(void) {
    _mkdir(dataDir);
}

void get_output_dir(char *buf) {
    sprintf(buf, "C:\\Users\\Public\\Desktop\\MTMS_Output");
    _mkdir(buf);
}

void get_output_file(char *buf, const char *filename) {
    char dir[MAX_PATH_LEN];
    get_output_dir(dir);
    sprintf(buf, "%s\\%s", dir, filename);
}

FILE* open_csv(const char *name, const char *mode) {
    char path[MAX_PATH_LEN];
    sprintf(path, "%s\\%s", dataDir, name);
    return fopen(path, mode);
}

void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) str[len-1] = '\0';
    if (len > 1 && str[len-2] == '\r') str[len-2] = '\0';
}

void get_datetime(char *buf, int len) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
}