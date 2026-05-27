#include "login.h"

User         userDB[MAX_USERS];
int          userCount = 0;
int          currentType = 0;
char         currentUser[50] = "";

PendingUser  pendingDB[MAX_USERS];
int          pendingCount = 0;

int parse_line_user(char *line, User *u) {
    char *p = strtok(line, ",");
    if (!p) return 0;
    strcpy(u->username, p);
    p = strtok(NULL, ",");
    if (!p) return 0;
    strcpy(u->password, p);
    p = strtok(NULL, ",");
    if (!p) return 0;
    u->type = atoi(p);
    return 1;
}

int parse_line_pending(char *line, PendingUser *p) {
    char *t = strtok(line, ",");
    if (!t) return 0;
    strcpy(p->username, t);
    t = strtok(NULL, ",");
    if (!t) return 0;
    strcpy(p->password, t);
    t = strtok(NULL, ",");
    if (!t) return 0;
    p->type = atoi(t);
    t = strtok(NULL, ",");
    if (!t) return 0;
    p->status = atoi(t);
    return 1;
}

void ensure_data_exists(void) {
    ensure_dir();
    FILE *fp = open_csv("users.csv", "r");
    if (!fp) {
        fp = open_csv("users.csv", "w");
        if (fp) {
            fprintf(fp, "admin,123456,1\n");
            fprintf(fp, "teacher1,abc123,2\n");
            fprintf(fp, "student1,pass789,3\n");
            fclose(fp);
            print_success("已创建默认用户数据。");
            printf("\n");
        }
    } else fclose(fp);
}

void ensure_pending_exists(void) {
    ensure_dir();
    FILE *fp = open_csv("pending_users.csv", "r");
    if (!fp) {
        fp = open_csv("pending_users.csv", "w");
        if (fp) fclose(fp);
    } else fclose(fp);
}

int load_users(void) {
    FILE *fp = open_csv("users.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    userCount = 0;
    while (fgets(line, MAX_LINE, fp) && userCount < MAX_USERS) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_line_user(line, &userDB[userCount])) userCount++;
    }
    fclose(fp);
    return 1;
}

int save_users(void) {
    FILE *fp = open_csv("users.csv", "w");
    if (!fp) return 0;
    for (int i = 0; i < userCount; i++)
        fprintf(fp, "%s,%s,%d\n", userDB[i].username, userDB[i].password, userDB[i].type);
    fclose(fp);
    return 1;
}

int load_pending_users(void) {
    FILE *fp = open_csv("pending_users.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    pendingCount = 0;
    while (fgets(line, MAX_LINE, fp) && pendingCount < MAX_USERS) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_line_pending(line, &pendingDB[pendingCount])) pendingCount++;
    }
    fclose(fp);
    return 1;
}

int save_pending_users(void) {
    FILE *fp = open_csv("pending_users.csv", "w");
    if (!fp) return 0;
    for (int i = 0; i < pendingCount; i++) {
        if (pendingDB[i].status == 0 || pendingDB[i].status == 1)
            fprintf(fp, "%s,%s,%d,%d\n", pendingDB[i].username, pendingDB[i].password,
                    pendingDB[i].type, pendingDB[i].status);
    }
    fclose(fp);
    return 1;
}

int do_login_check(const char *username, const char *password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDB[i].username, username) == 0 && strcmp(userDB[i].password, password) == 0) {
            currentType = userDB[i].type;
            strcpy(currentUser, userDB[i].username);
            return 1;
        }
    }
    return 0;
}

int check_pending_status(const char *username, const char *password) {
    for (int i = 0; i < pendingCount; i++) {
        if (strcmp(pendingDB[i].username, username) == 0 && strcmp(pendingDB[i].password, password) == 0)
            return pendingDB[i].status;
    }
    return -1;
}

int user_exists(const char *username) {
    for (int i = 0; i < userCount; i++) if (strcmp(userDB[i].username, username) == 0) return 1;
    return 0;
}

int pending_username_exists(const char *username) {
    for (int i = 0; i < pendingCount; i++)
        if (strcmp(pendingDB[i].username, username) == 0 && pendingDB[i].status == 0) return 1;
    return 0;
}

int register_user(const char *username, const char *password, int type) {
    for (int i = 0; i < pendingCount; i++) {
        if (strcmp(pendingDB[i].username, username) == 0 && pendingDB[i].status != 0) {
            strcpy(pendingDB[i].password, password);
            pendingDB[i].type = type;
            pendingDB[i].status = 0;
            return save_pending_users();
        }
    }
    if (pendingCount >= MAX_USERS) return 0;
    strcpy(pendingDB[pendingCount].username, username);
    strcpy(pendingDB[pendingCount].password, password);
    pendingDB[pendingCount].type = type;
    pendingDB[pendingCount].status = 0;
    pendingCount++;
    return save_pending_users();
}

const char* type_name(int type) {
    switch (type) {
        case 1: return "管理员";
        case 2: return "教师";
        case 3: return "学生";
        default: return "未知";
    }
}