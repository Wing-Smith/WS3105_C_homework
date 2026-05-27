#include "login.h"
#include <windows.h>

Device       deviceDB[MAX_DEVICES];
int          deviceCount = 0;

const char* device_type_name(int type) {
    switch (type) {
        case 0: return "车床";
        case 1: return "铣床";
        case 2: return "钳工台";
        case 3: return "焊机";
        default: return "未知";
    }
}

const char* danger_name(int d) {
    switch (d) {
        case 0: return "低";
        case 1: return "中";
        case 2: return "高";
        default: return "未知";
    }
}

static int parse_device(char *line, Device *d) {
    char *p = strtok(line, ",");
    if (!p) return 0; d->id = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(d->name, p);
    p = strtok(NULL, ","); if (!p) return 0; d->type = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(d->spec, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(d->range, p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(d->precision, p);
    p = strtok(NULL, ","); if (!p) return 0; d->total_hours = atof(p);
    p = strtok(NULL, ","); if (!p) return 0; d->use_count = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(d->status, p);
    p = strtok(NULL, ","); if (!p) return 0; d->danger = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(d->next_maintenance, p);
    return 1;
}

static void build_device_str(char *buf, Device *d) {
    sprintf(buf, "%d,%s,%d,%s,%s,%s,%.1f,%d,%s,%d,%s",
            d->id, d->name, d->type, d->spec, d->range, d->precision,
            d->total_hours, d->use_count, d->status, d->danger, d->next_maintenance);
}

void ensure_all_csv(void) {
    ensure_dir();
    const char *files[] = {"devices.csv","reservations.csv","maintenance.csv",
                           "safety_exam.csv","repair_orders.csv","work_hours.csv","operation_log.csv"};
    for (int i = 0; i < 7; i++) {
        FILE *fp = open_csv(files[i], "r");
        if (!fp) {
            fp = open_csv(files[i], "w");
            if (fp) fclose(fp);
        } else fclose(fp);
    }
    FILE *fp = open_csv("devices.csv", "r");
    if (fp) { fclose(fp); return; }
    fp = open_csv("devices.csv", "w");
    if (fp) {
        fprintf(fp, "1,普通车床,0,CA6140,0-1000mm,IT8,120.5,45,正常,2,2026-06-01\n");
        fprintf(fp, "2,立式铣床,1,X5032,0-800mm,IT7,85.0,32,正常,1,2026-07-15\n");
        fprintf(fp, "3,钳工台,2,PT-01,通用,手动,200.0,120,正常,0,2026-05-30\n");
        fprintf(fp, "4,氩弧焊机,3,WS-200,0-200A,--,60.0,18,正常,2,2026-06-10\n");
        fclose(fp);
    }
}

int load_devices(void) {
    FILE *fp = open_csv("devices.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    deviceCount = 0;
    while (fgets(line, MAX_LINE, fp) && deviceCount < MAX_DEVICES) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_device(line, &deviceDB[deviceCount])) deviceCount++;
    }
    fclose(fp);
    return 1;
}

int save_devices(void) {
    FILE *fp = open_csv("devices.csv", "w");
    if (!fp) return 0;
    char buf[MAX_LINE];
    for (int i = 0; i < deviceCount; i++) {
        build_device_str(buf, &deviceDB[i]);
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);
    return 1;
}

void list_devices(void) {
    print_divider();
    print_centered("设 备 列 表");
    print_divider();
    printf("\n");
    printf("  %-4s %-12s %-8s %-10s %-12s %-8s %-8s %-6s %-8s %-6s %-12s\n",
           "ID","名称","类型","型号","加工范围","精度","累计工时","使用次数","状态","危险","下次保养");
    print_divider();
    for (int i = 0; i < deviceCount; i++) {
        Device *d = &deviceDB[i];
        printf("  %-4d %-12s %-8s %-10s %-12s %-8s %-8.1f %-6d %-8s %-6s %-12s\n",
               d->id, d->name, device_type_name(d->type), d->spec, d->range, d->precision,
               d->total_hours, d->use_count, d->status, danger_name(d->danger), d->next_maintenance);
    }
}

void add_device(void) {
    if (deviceCount >= MAX_DEVICES) { print_error("设备数量已达上限"); return; }
    Device *d = &deviceDB[deviceCount];
    d->id = deviceCount + 1;

    apply_theme(current_theme);
    print_divider();
    print_centered("新 增 设 备");
    print_divider();
    printf("\n");

    print_prompt("设备名称: ");
    scanf("%s", d->name);

    int t;
    do {
        apply_theme(current_theme);
        print_divider();
        print_centered("选 择 设 备 类 型");
        print_divider();
        print_option(0, "车床");
        print_option(1, "铣床");
        print_option(2, "钳工台");
        print_option(3, "焊机");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &t);
    } while (t < 0 || t > 3);
    d->type = t;

    apply_theme(current_theme);
    print_divider();
    print_centered("填 写 参 数");
    print_divider();
    printf("\n");

    print_prompt("型号规格: ");
    scanf("%s", d->spec);
    print_prompt("加工范围 (如 0-1000mm): ");
    scanf("%s", d->range);
    print_prompt("精度等级 (车床填公差等级如IT7/IT8，钳工台填'手动'，焊机填'--'): ");
    scanf("%s", d->precision);
    d->total_hours = 0; d->use_count = 0;
    strcpy(d->status, "正常");

    int g;
    do {
        apply_theme(current_theme);
        print_divider();
        print_centered("选 择 危 险 等 级");
        print_divider();
        print_option(0, "低（低速转/无切削，如钳工台）");
        print_option(1, "中（一般旋转设备，如车床/铣床）");
        print_option(2, "高（高速旋转/切削/高压，如车床/焊机）");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &g);
    } while (g < 0 || g > 2);
    d->danger = g;

    apply_theme(current_theme);
    print_divider();
    print_centered("保 养 日 期");
    print_divider();
    printf("\n");
    print_prompt("下次保养日期(YYYY-MM-DD): ");
    scanf("%s", d->next_maintenance);
    deviceCount++;
    save_devices();
    print_success("设备已添加。");
}

void delete_device(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("删 除 设 备");
    print_divider();
    printf("\n");
    print_prompt("请输入要删除的设备ID: ");
    int id;
    scanf("%d", &id);
    int found = -1;
    for (int i = 0; i < deviceCount; i++) if (deviceDB[i].id == id) { found = i; break; }
    if (found < 0) { print_error("未找到该设备。"); return; }
    for (int i = found; i < deviceCount - 1; i++) deviceDB[i] = deviceDB[i+1];
    deviceCount--;
    save_devices();
    print_success("设备已删除。");
}

void device_maintenance_warn(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("保 养 预 警");
    print_divider();
    printf("\n");
    char today[20];
    print_prompt("请输入查询基准日期(YYYY-MM-DD): ");
    scanf("%s", today);
    printf("\n");
    int found = 0;
    for (int i = 0; i < deviceCount; i++) {
        if (strcmp(deviceDB[i].next_maintenance, today) <= 0) {
            char buf[128];
            sprintf(buf, "[!] %s (ID:%d) 保养已到期或超期，到期日: %s",
                   deviceDB[i].name, deviceDB[i].id, deviceDB[i].next_maintenance);
            print_warn(buf);
            found = 1;
        }
    }
    if (!found) print_centered("无到期保养设备。");
}

void device_utilization_report(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("设 备 利 用 率 统 计");
    print_divider();
    printf("\n");
    printf("  %-10s %-12s %-10s %-10s %-10s\n", "设备ID","设备名称","总预约数","活跃数","利用率");
    print_divider();
    for (int i = 0; i < deviceCount; i++) {
        int total = 0, active = 0;
        for (int j = 0; j < reserveCount; j++) {
            if (reserveDB[j].device_id == deviceDB[i].id && strcmp(reserveDB[j].status, "已取消") != 0) {
                total++;
                if (strcmp(reserveDB[j].status, "使用中") == 0) active++;
            }
        }
        double rate = (reserveCount > 0) ? (total * 100.0 / reserveCount) : 0;
        printf("  %-10d %-12s %-10d %-10d %-9.1f%%\n", deviceDB[i].id, deviceDB[i].name, total, active, rate);
    }
}

/* ========== PDF 导出 ========== */
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

void export_device_report(void) {
    if (deviceCount == 0) {
        print_warn("无设备数据，无法生成报表。");
        return;
    }

    int total[MAX_DEVICES] = {0};
    int active[MAX_DEVICES] = {0};
    int total_reserve_all = 0;
    double avg_rate = 0;

    for (int i = 0; i < deviceCount; i++) {
        for (int j = 0; j < reserveCount; j++) {
            if (reserveDB[j].device_id == deviceDB[i].id && strcmp(reserveDB[j].status, "已取消") != 0) {
                total[i]++;
                total_reserve_all++;
                if (strcmp(reserveDB[j].status, "使用中") == 0) active[i]++;
            }
        }
    }

    char texPath[MAX_PATH_LEN];
    char pdfPath[MAX_PATH_LEN];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char texName[64], pdfName[64];
    sprintf(texName, "device_report_%04d%02d%02d.tex", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    sprintf(pdfName, "device_report_%04d%02d%02d.pdf", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    get_output_file(texPath, texName);
    get_output_file(pdfPath, pdfName);

    FILE *fp = fopen(texPath, "wb");
    if (!fp) {
        print_error("无法创建设备报表文件，请检查是否使用了计算机管理员权限运行程序！");
        return;
    }

    U8F(fp, "\\documentclass{ctexart}\n");
    U8F(fp, "\\usepackage{geometry}\n");
    U8F(fp, "\\usepackage{array}\n");
    U8F(fp, "\\usepackage{booktabs}\n");
    U8F(fp, "\\geometry{a4paper, margin=2.5cm}\n");
    U8F(fp, "\\begin{document}\n");
    U8F(fp, "\\thispagestyle{empty}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "{\\LARGE\\bfseries 设备利用率统计报告}\\\\[0.3cm]\n");

    char info[256];
    sprintf(info, "{\\large 统计日期：%04d-%02d-%02d | 设备总数：%d}\\\\[0.5cm]\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, deviceCount);
    U8F(fp, info);
    U8F(fp, "\\end{center}\n");

    U8F(fp, "\\renewcommand{\\arraystretch}{1.2}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "\\begin{tabular}{|>{\\centering\\arraybackslash}p{1.5cm}|>{\\centering\\arraybackslash}p{3cm}|>{\\centering\\arraybackslash}p{2cm}|>{\\centering\\arraybackslash}p{2cm}|>{\\centering\\arraybackslash}p{2cm}|>{\\centering\\arraybackslash}p{2cm}|}\n");
    U8F(fp, "\\hline\n");
    U8F(fp, "\\textbf{设备ID} & \\textbf{设备名称} & \\textbf{类型} & \\textbf{总预约} & \\textbf{活跃数} & \\textbf{利用率} \\\\\n");
    U8F(fp, "\\hline\n");

    for (int i = 0; i < deviceCount; i++) {
        double rate = (reserveCount > 0) ? (total[i] * 100.0 / reserveCount) : 0;
        avg_rate += rate;

        char *u8_name = gbk_to_utf8(deviceDB[i].name);
        char *u8_type = gbk_to_utf8(device_type_name(deviceDB[i].type));

        fprintf(fp, "%d & %s & %s & %d & %d & %.1f\\%% \\\\\n",
                deviceDB[i].id,
                u8_name ? u8_name : deviceDB[i].name,
                u8_type ? u8_type : device_type_name(deviceDB[i].type),
                total[i], active[i], rate);
        U8F(fp, "\\hline\n");

        free(u8_name);
        free(u8_type);
    }

    U8F(fp, "\\end{tabular}\n");
    U8F(fp, "\\end{center}\n");

    U8F(fp, "\\vspace{0.5cm}\n");
    U8F(fp, "\\noindent\n");
    U8F(fp, "\\begin{tabular}{ll}\n");
    sprintf(info, "\\textbf{设备总数} & %d 台 \\\\\n", deviceCount);
    U8F(fp, info);
    sprintf(info, "\\textbf{平均利用率} & %.1f\\%% \\\\\n", deviceCount > 0 ? avg_rate / deviceCount : 0);
    U8F(fp, info);
    sprintf(info, "\\textbf{总预约次数} & %d 次 \\\\\n", total_reserve_all);
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
        print_success("设备利用率报表PDF已生成：");
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