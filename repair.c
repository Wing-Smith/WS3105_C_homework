#include "login.h"
#include <windows.h>

RepairOrder  repairDB[MAX_REPAIR];
int          repairCount = 0;

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

static int parse_repair(char *line, RepairOrder *r) {
    char *p = strtok(line, ",");
    if (!p) return 0; r->id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; r->device_id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->reporter, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->date, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->description, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->status, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->assignee, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(r->complete_date, p);
    return 1;
}

static void build_repair_str(char *buf, RepairOrder *r) {
    sprintf(buf, "%d,%d,%s,%s,%s,%s,%s,%s",
            r->id, r->device_id, r->reporter, r->date, r->description, r->status, r->assignee, r->complete_date);
}

int load_repairs(void) {
    FILE *fp = open_csv("repair_orders.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    repairCount = 0;
    while (fgets(line, MAX_LINE, fp) && repairCount < MAX_REPAIR) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_repair(line, &repairDB[repairCount])) repairCount++;
    }
    fclose(fp);
    return 1;
}

int save_repairs(void) {
    FILE *fp = open_csv("repair_orders.csv", "w");
    if (!fp) return 0;
    char buf[MAX_LINE];
    for (int i = 0; i < repairCount; i++) {
        build_repair_str(buf, &repairDB[i]);
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);
    return 1;
}

void list_repairs(void) {
    printf("\n");
    printf("  %-4s %-8s %-12s %-12s %-20s %-8s %-10s %-12s\n",
           "ID","设备ID","上报人","日期","描述","状态","受理人","完工日期");
    print_divider();
    for (int i = 0; i < repairCount; i++) {
        RepairOrder *r = &repairDB[i];
        printf("  %-4d %-8d %-12s %-12s %-20s %-8s %-10s %-12s\n",
               r->id, r->device_id, r->reporter, r->date, r->description,
               r->status, r->assignee, r->complete_date);
    }
}

void report_repair(void) {
    if (repairCount >= MAX_REPAIR) { print_error("记录数量已满"); return; }
    RepairOrder *r = &repairDB[repairCount];
    r->id = repairCount + 1;

    apply_theme(current_theme);
    print_divider();
    print_centered("故 障 上 报");
    print_divider();
    printf("\n");

    list_devices();
    printf("\n");
    print_prompt("请输入故障设备ID: ");
    scanf("%d", &r->device_id);
    strcpy(r->reporter, currentUser);
    get_datetime(r->date, sizeof(r->date));
    print_prompt("故障描述: ");
    scanf(" %[^\n]", r->description);
    strcpy(r->status, "待受理");
    strcpy(r->assignee, "无");
    strcpy(r->complete_date, "未完工");
    repairCount++;
    save_repairs();
    print_success("报修单已提交。");
}

void assign_repair(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("受 理 报 修");
    print_divider();
    printf("\n");
    list_repairs();
    printf("\n");
    print_prompt("请输入要受理的报修单ID: ");
    int id;
    scanf("%d", &id);
    for (int i = 0; i < repairCount; i++) {
        if (repairDB[i].id == id && strcmp(repairDB[i].status, "待受理") == 0) {
            print_prompt("受理人: ");
            scanf("%s", repairDB[i].assignee);
            strcpy(repairDB[i].status, "维修中");
            save_repairs();
            print_success("已受理。");
            return;
        }
    }
    print_error("未找到该报修单或状态不可受理。");
}

void complete_repair(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("完 工 确 认");
    print_divider();
    printf("\n");
    list_repairs();
    printf("\n");
    print_prompt("请输入要完工的报修单ID: ");
    int id;
    scanf("%d", &id);
    for (int i = 0; i < repairCount; i++) {
        if (repairDB[i].id == id && strcmp(repairDB[i].status, "维修中") == 0) {
            get_datetime(repairDB[i].complete_date, sizeof(repairDB[i].complete_date));
            strcpy(repairDB[i].status, "已完工");
            save_repairs();
            print_success("已确认完工。");
            return;
        }
    }
    print_error("未找到该报修单或状态不可完工。");
}

/* ========== PDF 导出 ========== */
void export_repair_report(void) {
    if (repairCount == 0) {
        print_warn("无报修记录，无法生成报表。");
        return;
    }

    int pending = 0, repairing = 0, done = 0;
    for (int i = 0; i < repairCount; i++) {
        if (strcmp(repairDB[i].status, "待受理") == 0) pending++;
        else if (strcmp(repairDB[i].status, "维修中") == 0) repairing++;
        else if (strcmp(repairDB[i].status, "已完工") == 0) done++;
    }

    char texPath[MAX_PATH_LEN];
    char pdfPath[MAX_PATH_LEN];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char texName[64], pdfName[64];
    sprintf(texName, "repair_report_%04d%02d%02d.tex", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    sprintf(pdfName, "repair_report_%04d%02d%02d.pdf", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    get_output_file(texPath, texName);
    get_output_file(pdfPath, pdfName);

    FILE *fp = fopen(texPath, "wb");
    if (!fp) {
        print_error("无法创建报修报表文件，请检查是否使用了计算机管理员权限运行程序！");
        return;
    }

    U8F(fp, "\\documentclass{ctexart}\n");
    U8F(fp, "\\usepackage{geometry}\n");
    U8F(fp, "\\usepackage{array}\n");
    U8F(fp, "\\usepackage{booktabs}\n");
    U8F(fp, "\\usepackage{xcolor}\n");
    U8F(fp, "\\geometry{a4paper, margin=2cm}\n");
    U8F(fp, "\\begin{document}\n");
    U8F(fp, "\\thispagestyle{empty}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "{\\LARGE\\bfseries 报修工单汇总报告}\\\\[0.3cm]\n");

    char info[256];
    sprintf(info, "{\\large 统计日期：%04d-%02d-%02d | 工单总数：%d}\\\\[0.5cm]\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, repairCount);
    U8F(fp, info);
    U8F(fp, "\\end{center}\n");

    U8F(fp, "\\noindent\n");
    U8F(fp, "\\begin{tabular}{ll}\n");
    sprintf(info, "\\textbf{待受理} & %d 条 \\\\\n", pending);
    U8F(fp, info);
    sprintf(info, "\\textbf{维修中} & %d 条 \\\\\n", repairing);
    U8F(fp, info);
    sprintf(info, "\\textbf{已完工} & %d 条 \\\\\n", done);
    U8F(fp, info);
    U8F(fp, "\\end{tabular}\n");
    U8F(fp, "\\vspace{0.5cm}\n");

    U8F(fp, "\\renewcommand{\\arraystretch}{1.2}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "\\begin{tabular}{|>{\\centering\\arraybackslash}p{1cm}|>{\\centering\\arraybackslash}p{1.2cm}|>{\\centering\\arraybackslash}p{1.5cm}|>{\\centering\\arraybackslash}p{1.8cm}|p{4cm}|>{\\centering\\arraybackslash}p{1.2cm}|>{\\centering\\arraybackslash}p{1.5cm}|>{\\centering\\arraybackslash}p{1.8cm}|}\n");
    U8F(fp, "\\hline\n");
    U8F(fp, "\\textbf{ID} & \\textbf{设备ID} & \\textbf{上报人} & \\textbf{日期} & \\textbf{故障描述} & \\textbf{状态} & \\textbf{受理人} & \\textbf{完工日期} \\\\\n");
    U8F(fp, "\\hline\n");

    for (int i = 0; i < repairCount; i++) {
        RepairOrder *r = &repairDB[i];
        char esc_desc[256];
        latex_escape(esc_desc, r->description, sizeof(esc_desc));

        char *u8_rep  = gbk_to_utf8(r->reporter);
        char *u8_stat = gbk_to_utf8(r->status);
        char *u8_ass  = gbk_to_utf8(r->assignee);
        char *u8_comp = gbk_to_utf8(r->complete_date);
        char *u8_date = gbk_to_utf8(r->date);
        char *u8_desc = gbk_to_utf8(esc_desc);

        fprintf(fp, "%d & %d & %s & %s & %s & %s & %s & %s \\\\\n",
                r->id, r->device_id,
                u8_rep ? u8_rep : r->reporter,
                u8_date ? u8_date : r->date,
                u8_desc ? u8_desc : esc_desc,
                u8_stat ? u8_stat : r->status,
                u8_ass ? u8_ass : r->assignee,
                u8_comp ? u8_comp : r->complete_date);
        U8F(fp, "\\hline\n");

        free(u8_rep); free(u8_stat); free(u8_ass); free(u8_comp); free(u8_date); free(u8_desc);
    }

    U8F(fp, "\\end{tabular}\n");
    U8F(fp, "\\end{center}\n");

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
        print_success("报修工单报表PDF已生成：");
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