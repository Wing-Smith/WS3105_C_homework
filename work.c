#include "login.h"
#include <windows.h>

/* GBK 转 UTF-8 */
static char* gbk_to_utf8(const char *gbk) {
    if (!gbk) return NULL;
    int wlen = MultiByteToWideChar(CP_ACP, 0, gbk, -1, NULL, 0);
    if (wlen <= 0) return NULL;
    wchar_t *wstr = (wchar_t*)malloc(wlen * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, gbk, -1, wstr, wlen);
    int ulen = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *utf8 = (char*)malloc(ulen);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, ulen, NULL, NULL);
    free(wstr);
    return utf8;
}

#define U8F(fp, str) do { \
    char *__u = gbk_to_utf8(str); \
    if (__u) { fputs(__u, fp); free(__u); } \
} while (0)

WorkHour     workDB[MAX_WORK];
int          workCount = 0;

static int parse_work(char *line, WorkHour *w) {
    char *p = strtok(line, ",");
    if (!p) return 0; strcpy(w->username, p);
    p = strtok(NULL, ","); if (!p) return 0; w->device_id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(w->date, p);
    p = strtok(NULL, ","); if (!p) return 0; w->duration = atof(p);
    p = strtok(NULL, ","); if (!p) return 0; w->score = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(w->comment, p);
    return 1;
}

int load_work_hours(void) {
    FILE *fp = open_csv("work_hours.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    workCount = 0;
    while (fgets(line, MAX_LINE, fp) && workCount < MAX_WORK) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_work(line, &workDB[workCount])) workCount++;
    }
    fclose(fp);
    return 1;
}

int save_work_hours(void) {
    FILE *fp = open_csv("work_hours.csv", "w");
    if (!fp) return 0;
    for (int i = 0; i < workCount; i++)
        fprintf(fp, "%s,%d,%s,%.1f,%d,%s\n", workDB[i].username, workDB[i].device_id,
                workDB[i].date, workDB[i].duration, workDB[i].score, workDB[i].comment);
    fclose(fp);
    return 1;
}

void list_work_hours(void) {
    printf("\n");
    printf("  %-15s %-8s %-12s %-8s %-6s %-20s\n",
           "用户名","设备ID","日期","工时","成绩","评语");
    print_divider();
    for (int i = 0; i < workCount; i++) {
        WorkHour *w = &workDB[i];
        printf("  %-15s %-8d %-12s %-8.1f %-6d %-20s\n",
               w->username, w->device_id, w->date, w->duration, w->score, w->comment);
    }
}

void add_work_hour(void) {
    if (workCount >= MAX_WORK) { print_error("记录数量已满"); return; }
    WorkHour *w = &workDB[workCount];

    apply_theme(current_theme);
    print_divider();
    print_centered("录 入 成 绩 / 工 时");
    print_divider();
    printf("\n");

    list_devices();
    printf("\n");
    print_prompt("请输入设备ID: ");
    int did;
    scanf("%d", &did);
    int found = 0;
    for (int i = 0; i < deviceCount; i++) if (deviceDB[i].id == did) { found = 1; break; }
    if (!found) { print_error("设备ID不存在。"); return; }
    w->device_id = did;

    print_prompt("用户名: ");
    scanf("%s", w->username);
    print_prompt("日期(YYYY-MM-DD): ");
    scanf("%s", w->date);
    print_prompt("实习工时(小时): ");
    scanf("%lf", &w->duration);
    print_prompt("成绩: ");
    scanf("%d", &w->score);
    print_prompt("评语: ");
    scanf(" %[^\n]", w->comment);
    workCount++;
    save_work_hours();
    print_success("记录已添加。");
}

static void latex_escape(char *dst, const char *src, int max) {
    int j = 0;
    for (int i = 0; src[i] && j < max - 2; i++) {
        if (strchr("%&_$#{}~^\\", src[i]) != NULL) {
            if (j < max - 2) { dst[j++] = '\\'; dst[j++] = src[i]; }
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

static void generate_work_report(const char *filter) {
    int filtered[MAX_WORK];
    int fcount = 0;
    int is_all = (strcmp(filter, "all") == 0 || strlen(filter) == 0);

    for (int i = 0; i < workCount; i++) {
        if (is_all || strstr(workDB[i].username, filter) != NULL) {
            filtered[fcount++] = i;
        }
    }

    if (fcount == 0) {
        print_warn("无符合条件的记录，无法生成报表。");
        return;
    }

    double total_hours = 0;
    int total_score = 0;
    for (int i = 0; i < fcount; i++) {
        total_hours += workDB[filtered[i]].duration;
        total_score += workDB[filtered[i]].score;
    }

    char texPath[MAX_PATH_LEN];
    char pdfPath[MAX_PATH_LEN];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char texName[64], pdfName[64];
    sprintf(texName, "work_report_%04d%02d%02d.tex", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    sprintf(pdfName, "work_report_%04d%02d%02d.pdf", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    get_output_file(texPath, texName);
    get_output_file(pdfPath, pdfName);

    FILE *fp = fopen(texPath, "wb");
    if (!fp) {
        print_error("无法创建报表文件，请检查是否使用了计算机管理员权限运行程序！");
        return;
    }

    /* LaTeX 头部 */
    U8F(fp, "\\documentclass{ctexart}\n");
    U8F(fp, "\\usepackage{geometry}\n");
    U8F(fp, "\\usepackage{array}\n");
    U8F(fp, "\\usepackage{booktabs}\n");
    U8F(fp, "\\geometry{a4paper, margin=2cm}\n");
    U8F(fp, "\\begin{document}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "{\\LARGE\\bfseries 金工实习工时与成绩统计报表}\\\\[0.3cm]\n");

    char info[256];
    if (is_all) {
        sprintf(info, "{\\large 筛选条件：全部记录 | 共 %d 条}\\\\[0.5cm]\n", fcount);
    } else {
        sprintf(info, "{\\large 筛选条件：关键字 \\\"%s\\\" | 共 %d 条}\\\\[0.5cm]\n", filter, fcount);
    }
    U8F(fp, info);
    U8F(fp, "\\end{center}\n");

    /* 表格：所有列固定宽度 */
    U8F(fp, "\\renewcommand{\\arraystretch}{1.2}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "\\begin{tabular}{|>{\\centering\\arraybackslash}p{1cm}|>{\\centering\\arraybackslash}p{2cm}|>{\\centering\\arraybackslash}p{1.5cm}|>{\\centering\\arraybackslash}p{2cm}|>{\\centering\\arraybackslash}p{1.5cm}|>{\\centering\\arraybackslash}p{1cm}|p{5cm}|}\n");
    U8F(fp, "\\hline\n");
    U8F(fp, "\\textbf{序号} & \\textbf{用户名} & \\textbf{设备ID} & \\textbf{日期} & \\textbf{工时(h)} & \\textbf{成绩} & \\textbf{评语} \\\\\n");
    U8F(fp, "\\hline\n");

    for (int i = 0; i < fcount; i++) {
        WorkHour *w = &workDB[filtered[i]];
        char esc_comment[256];
        latex_escape(esc_comment, w->comment, sizeof(esc_comment));
        char *u8_user = gbk_to_utf8(w->username);
        char *u8_date = gbk_to_utf8(w->date);
        char *u8_comm = gbk_to_utf8(esc_comment);

        fprintf(fp, "%d & %s & %d & %s & %.1f & %d & %s \\\\\n",
                i + 1,
                u8_user ? u8_user : w->username,
                w->device_id,
                u8_date ? u8_date : w->date,
                w->duration,
                w->score,
                u8_comm ? u8_comm : esc_comment);
        U8F(fp, "\\hline\n");

        free(u8_user);
        free(u8_date);
        free(u8_comm);
    }

    U8F(fp, "\\end{tabular}\n");
    U8F(fp, "\\end{center}\n");

    /* 统计汇总 */
    U8F(fp, "\\vspace{0.5cm}\n");
    U8F(fp, "\\noindent\n");
    U8F(fp, "\\begin{tabular}{ll}\n");
    sprintf(info, "\\textbf{总工时} & %.1f 小时 \\\\\n", total_hours);
    U8F(fp, info);
    sprintf(info, "\\textbf{平均成绩} & %.1f 分 \\\\\n", fcount > 0 ? (double)total_score / fcount : 0);
    U8F(fp, info);
    sprintf(info, "\\textbf{记录条数} & %d 条 \\\\\n", fcount);
    U8F(fp, info);
    U8F(fp, "\\end{tabular}\n");

    U8F(fp, "\\vspace{1cm}\n");
    U8F(fp, "\\begin{flushright}\n");
    U8F(fp, "\\textbf{昆明理工大学实训中心}\\\\[0.3cm]\n");
    sprintf(info, "%04d年%02d月%02d日\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    U8F(fp, info);
    U8F(fp, "\\end{flushright}\n");
    U8F(fp, "\\end{document}\n");

    fclose(fp);

    char cmd[512];
    char outDir[MAX_PATH_LEN];
    get_output_dir(outDir);
    sprintf(cmd, "pdflatex -interaction=nonstopmode -output-directory=\"%s\" \"%s\" > nul 2>&1", outDir, texPath);
    int ret = system(cmd);

    if (ret == 0) {
        print_success("报表PDF已生成：");
        char msg[256];
        sprintf(msg, "文件位置: %s", pdfPath);
        print_centered(msg);
    } else {
        print_warn("PDF自动编译失败，可能未安装TeX Live/MiKTeX。");
        char msg1[256], msg2[256];
        sprintf(msg1, "已生成LaTeX源文件: %s", texPath);
        sprintf(msg2, "请手动编译: pdflatex \"%s\"", texPath);
        print_centered(msg1);
        print_centered(msg2);
    }
}

void export_work_hours(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("工 时 与 成 绩 统 计 导 出");
    print_divider();
    printf("\n");
    char cls[50];
    print_prompt("请输入班级/查询关键字(输入all表示全部): ");
    scanf("%s", cls);
    printf("\n");

    /* 终端预览 */
    printf("  %-15s %-8s %-12s %-8s %-6s %-20s\n",
           "用户名","设备ID","日期","工时","成绩","评语");
    print_divider();
    int shown = 0;
    for (int i = 0; i < workCount; i++) {
        if (strcmp(cls, "all") == 0 || strstr(workDB[i].username, cls) != NULL) {
            WorkHour *w = &workDB[i];
            printf("  %-15s %-8d %-12s %-8.1f %-6d %-20s\n",
                   w->username, w->device_id, w->date, w->duration, w->score, w->comment);
            shown++;
        }
    }
    if (shown == 0) {
        print_warn("无符合条件的记录。");
        return;
    }

    printf("\n");
    print_prompt("确认生成PDF报表? (1=确认, 0=取消): ");
    int confirm;
    scanf("%d", &confirm);
    if (confirm != 1) {
        print_centered("已取消导出。");
        return;
    }

    generate_work_report(cls);
}