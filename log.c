#include "login.h"

OpLog        logDB[MAX_LOG];
int          logCount = 0;

static int parse_log(char *line, OpLog *l) {
    char *p = strtok(line, ",");
    if (!p) return 0; strcpy(l->timestamp, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(l->username, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(l->action, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(l->detail, p);
    return 1;
}

int load_logs(void) {
    FILE *fp = open_csv("operation_log.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    logCount = 0;
    while (fgets(line, MAX_LINE, fp) && logCount < MAX_LOG) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_log(line, &logDB[logCount])) logCount++;
    }
    fclose(fp);
    return 1;
}

int save_logs(void) {
    FILE *fp = open_csv("operation_log.csv", "w");
    if (!fp) return 0;
    for (int i = 0; i < logCount; i++)
        fprintf(fp, "%s,%s,%s,%s\n", logDB[i].timestamp, logDB[i].username,
                logDB[i].action, logDB[i].detail);
    fclose(fp);
    return 1;
}

void list_logs(void) {
    printf("\n");
    printf("  %-20s %-15s %-15s %-30s\n", "时间","用户名","动作","详情");
    print_divider();
    for (int i = 0; i < logCount; i++) {
        OpLog *l = &logDB[i];
        printf("  %-20s %-15s %-15s %-30s\n", l->timestamp, l->username, l->action, l->detail);
    }
}

void add_log(const char *username, const char *action, const char *detail) {
    if (logCount >= MAX_LOG) {
        for (int i = 0; i < MAX_LOG - 1; i++) logDB[i] = logDB[i+1];
        logCount = MAX_LOG - 1;
    }
    OpLog *l = &logDB[logCount];
    get_datetime(l->timestamp, sizeof(l->timestamp));
    strcpy(l->username, username);
    strcpy(l->action, action);
    strcpy(l->detail, detail);
    logCount++;
    save_logs();
}