#include "login.h"

Reservation  reserveDB[MAX_RESERVE];
int          reserveCount = 0;

static int parse_reserve(char *line, Reservation *r) {
    char *p = strtok(line, ",");
    if (!p) return 0; r->id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; r->device_id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->username, p);
    p = strtok(NULL, ","); if (!p) return 0; r->user_type = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->date, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->time_slot, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->purpose, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->priority, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->status, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->create_time, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->checkin_time, p);
    return 1;
}

static void build_reserve_str(char *buf, Reservation *r) {
    sprintf(buf, "%d,%d,%s,%d,%s,%s,%s,%s,%s,%s,%s",
            r->id, r->device_id, r->username, r->user_type, r->date, r->time_slot,
            r->purpose, r->priority, r->status, r->create_time, r->checkin_time);
}

int load_reservations(void) {
    FILE *fp = open_csv("reservations.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    reserveCount = 0;
    while (fgets(line, MAX_LINE, fp) && reserveCount < MAX_RESERVE) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_reserve(line, &reserveDB[reserveCount])) reserveCount++;
    }
    fclose(fp);
    return 1;
}

int save_reservations(void) {
    FILE *fp = open_csv("reservations.csv", "w");
    if (!fp) return 0;
    char buf[MAX_LINE];
    for (int i = 0; i < reserveCount; i++) {
        build_reserve_str(buf, &reserveDB[i]);
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);
    return 1;
}

void list_my_reservations(const char *username) {
    print_divider();
    print_centered("我 的 预 约");
    print_divider();
    printf("\n");
    printf("  %-4s %-10s %-8s %-12s %-10s %-8s %-8s %-12s\n",
           "ID","设备ID","日期","时段","用途","优先级","状态","签到时间");
    print_divider();
    for (int i = 0; i < reserveCount; i++) {
        if (strcmp(reserveDB[i].username, username) != 0) continue;
        Reservation *r = &reserveDB[i];
        printf("  %-4d %-10d %-8s %-12s %-10s %-8s %-8s %-12s\n",
               r->id, r->device_id, r->date, r->time_slot, r->purpose,
               r->priority, r->status, r->checkin_time);
    }
}

int check_time_conflict(int device_id, const char *date, const char *time_slot, int exclude_id) {
    for (int i = 0; i < reserveCount; i++) {
        if (reserveDB[i].id == exclude_id) continue;
        if (reserveDB[i].device_id == device_id && strcmp(reserveDB[i].date, date) == 0 &&
            strcmp(reserveDB[i].status, "已取消") != 0) {
            if (strcmp(reserveDB[i].time_slot, time_slot) == 0) return 1;
        }
    }
    return 0;
}

int check_safety_pass(const char *username, int device_type) {
    for (int i = 0; i < safetyCount; i++) {
        if (strcmp(safetyDB[i].username, username) == 0 &&
            safetyDB[i].device_type == device_type && safetyDB[i].passed)
            return 1;
    }
    return 0;
}

int add_reservation(const char *username, int user_type) {
    if (reserveCount >= MAX_RESERVE) { print_error("预约数量已达上限"); return 0; }
    list_devices();
    printf("\n");
    print_prompt("请输入要预约的设备ID: ");
    int did;
    scanf("%d", &did);
    int found = -1;
    for (int i = 0; i < deviceCount; i++) if (deviceDB[i].id == did) { found = i; break; }
    if (found < 0) { print_error("设备不存在。"); return 0; }

    if (deviceDB[found].danger == 2) {
        if (!check_safety_pass(username, deviceDB[found].type)) {
            print_error("您未通过该危险等级设备的安全考试，无法预约。");
            return 0;
        }
    }

    Reservation *r = &reserveDB[reserveCount];
    r->id = reserveCount + 1;
    r->device_id = did;
    strcpy(r->username, username);
    r->user_type = user_type;

    apply_theme(current_theme);
    print_divider();
    print_centered("填 写 预 约 信 息");
    print_divider();
    printf("\n");

    print_prompt("预约日期(YYYY-MM-DD): ");
    scanf("%s", r->date);
    print_prompt("预约时段(如08:00-10:00): ");
    scanf("%s", r->time_slot);

    int purpose_choice;
    do {
        apply_theme(current_theme);
        print_divider();
        print_centered("选 择 预 约 用 途");
        print_divider();
        print_option(1, "课程实训");
        print_option(2, "自主练习");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &purpose_choice);
    } while (purpose_choice < 1 || purpose_choice > 2);
    if (purpose_choice == 1) {
        strcpy(r->purpose, "课程实训");
        strcpy(r->priority, "高");
    } else {
        strcpy(r->purpose, "自主练习");
        strcpy(r->priority, "低");
    }

    if (check_time_conflict(did, r->date, r->time_slot, -1)) {
        print_error("该时段已被预约，存在冲突。");
        return 0;
    }

    strcpy(r->status, "已预约");
    get_datetime(r->create_time, sizeof(r->create_time));
    strcpy(r->checkin_time, "未签到");
    reserveCount++;
    save_reservations();
    print_success("预约成功。");
    return 1;
}

void cancel_reservation(const char *username) {
    apply_theme(current_theme);
    list_my_reservations(username);
    printf("\n");
    print_prompt("请输入要取消的预约ID: ");
    int id;
    scanf("%d", &id);
    for (int i = 0; i < reserveCount; i++) {
        if (reserveDB[i].id == id && strcmp(reserveDB[i].username, username) == 0) {
            strcpy(reserveDB[i].status, "已取消");
            save_reservations();
            print_success("预约已取消。");
            return;
        }
    }
    print_error("未找到该预约或无权限。");
}

void checkin_reservation(const char *username) {
    apply_theme(current_theme);
    list_my_reservations(username);
    printf("\n");
    print_prompt("请输入要签到的预约ID: ");
    int id;
    scanf("%d", &id);
    for (int i = 0; i < reserveCount; i++) {
        if (reserveDB[i].id == id && strcmp(reserveDB[i].username, username) == 0
            && strcmp(reserveDB[i].status, "已预约") == 0) {
            get_datetime(reserveDB[i].checkin_time, sizeof(reserveDB[i].checkin_time));
            strcpy(reserveDB[i].status, "使用中");
            save_reservations();
            print_success("签到成功，开始计时。");
            return;
        }
    }
    print_error("未找到该预约或状态不可签到。");
}

void auto_release_overdue(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("超 时 释 放 检 查");
    print_divider();
    printf("\n");
    char today[30];
    get_datetime(today, sizeof(today));
    int found = 0;
    for (int i = 0; i < reserveCount; i++) {
        if (strcmp(reserveDB[i].status, "已预约") == 0 && strcmp(reserveDB[i].checkin_time, "未签到") == 0) {
            char buf[128];
            sprintf(buf, "[!] 预约ID %d (%s) 未签到，请注意是否超时", reserveDB[i].id, reserveDB[i].create_time);
            print_warn(buf);
            found = 1;
        }
    }
    if (!found) print_centered("无超时预约。");
}