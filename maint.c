#include "login.h"

Maintenance  maintDB[MAX_MAINT];
int          maintCount = 0;

static int parse_maint(char *line, Maintenance *m) {
    char *p = strtok(line, ",");
    if (!p) return 0; m->id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; m->device_id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(m->date, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(m->content, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(m->next_date, p);
    return 1;
}

static void build_maint_str(char *buf, Maintenance *m) {
    sprintf(buf, "%d,%d,%s,%s,%s", m->id, m->device_id, m->date, m->content, m->next_date);
}

int load_maintenance(void) {
    FILE *fp = open_csv("maintenance.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    maintCount = 0;
    while (fgets(line, MAX_LINE, fp) && maintCount < MAX_MAINT) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_maint(line, &maintDB[maintCount])) maintCount++;
    }
    fclose(fp);
    return 1;
}

int save_maintenance(void) {
    FILE *fp = open_csv("maintenance.csv", "w");
    if (!fp) return 0;
    char buf[MAX_LINE];
    for (int i = 0; i < maintCount; i++) {
        build_maint_str(buf, &maintDB[i]);
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);
    return 1;
}

void list_maintenance(void) {
    printf("\n");
    printf("  %-4s %-8s %-12s %-20s %-12s\n", "ID","设备ID","日期","内容","下次保养");
    print_divider();
    for (int i = 0; i < maintCount; i++) {
        Maintenance *m = &maintDB[i];
        printf("  %-4d %-8d %-12s %-20s %-12s\n", m->id, m->device_id, m->date, m->content, m->next_date);
    }
}

void add_maintenance(void) {
    if (maintCount >= MAX_MAINT) { print_error("记录数量已满"); return; }
    Maintenance *m = &maintDB[maintCount];
    m->id = maintCount + 1;

    apply_theme(current_theme);
    print_divider();
    print_centered("新 增 保 养 记 录");
    print_divider();
    printf("\n");

    list_devices();
    printf("\n");
    print_prompt("请输入要保养的设备ID: ");
    scanf("%d", &m->device_id);
    print_prompt("保养日期(YYYY-MM-DD): ");
    scanf("%s", m->date);
    print_prompt("保养内容: ");
    scanf(" %[^\n]", m->content);
    print_prompt("下次保养日期(YYYY-MM-DD): ");
    scanf("%s", m->next_date);
    maintCount++;
    save_maintenance();
    for (int i = 0; i < deviceCount; i++) {
        if (deviceDB[i].id == m->device_id) {
            strcpy(deviceDB[i].next_maintenance, m->next_date);
            save_devices();
            break;
        }
    }
    print_success("保养记录已添加。");
}