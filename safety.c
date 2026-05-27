#include "login.h"
#include <windows.h>

SafetyExam   safetyDB[MAX_SAFETY];
int          safetyCount = 0;

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

static int parse_safety(char *line, SafetyExam *s) {
    char *p = strtok(line, ",");
    if (!p) return 0; strcpy(s->username, p);
    p = strtok(NULL, ","); if (!p) return 0; s->device_type = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; s->passed = atoi(p);
    p = strtok(NULL, ","); if (!p) return 0; strcpy(s->exam_date, p);
    return 1;
}

int load_safety(void) {
    FILE *fp = open_csv("safety_exam.csv", "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    safetyCount = 0;
    while (fgets(line, MAX_LINE, fp) && safetyCount < MAX_SAFETY) {
        trim_newline(line);
        if (strlen(line) > 0 && parse_safety(line, &safetyDB[safetyCount])) safetyCount++;
    }
    fclose(fp);
    return 1;
}

int save_safety(void) {
    FILE *fp = open_csv("safety_exam.csv", "w");
    if (!fp) return 0;
    for (int i = 0; i < safetyCount; i++)
        fprintf(fp, "%s,%d,%d,%s\n", safetyDB[i].username, safetyDB[i].device_type,
                safetyDB[i].passed, safetyDB[i].exam_date);
    fclose(fp);
    return 1;
}

void list_safety(void) {
    printf("\n");
    printf("  %-15s %-12s %-8s %-12s\n", "用户名","设备类型","状态","考试日期");
    print_divider();
    for (int i = 0; i < safetyCount; i++) {
        SafetyExam *s = &safetyDB[i];
        printf("  %-15s %-12s %-8s %-12s\n", s->username, device_type_name(s->device_type),
               s->passed ? "合格" : "不合格", s->exam_date);
    }
}

void add_safety_record(void) {
    if (safetyCount >= MAX_SAFETY) { print_error("记录数量已满"); return; }
    SafetyExam *s = &safetyDB[safetyCount];

    apply_theme(current_theme);
    print_divider();
    print_centered("新 增 安 全 考 试 记 录");
    print_divider();
    printf("\n");

    print_prompt("用户名: ");
    scanf("%s", s->username);

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
    s->device_type = t;

    apply_theme(current_theme);
    print_divider();
    print_centered("选 择 考 试 结 果");
    print_divider();
    print_option(0, "不合格");
    print_option(1, "合格");
    printf("\n");
    print_prompt("请选择: ");
    int p;
    scanf("%d", &p);
    s->passed = p;

    apply_theme(current_theme);
    print_divider();
    print_centered("考 试 日 期");
    print_divider();
    printf("\n");
    print_prompt("考试日期(YYYY-MM-DD): ");
    scanf("%s", s->exam_date);
    safetyCount++;
    save_safety();
    print_success("记录已添加。");
}

void remove_safety_record(void) {
    apply_theme(current_theme);
    print_divider();
    print_centered("删 除 安 全 考 试");
    print_divider();
    printf("\n");

    char username[50];
    int dtype;
    print_prompt("用户名: ");
    scanf("%s", username);

    do {
        apply_theme(current_theme);
        print_divider();
        print_centered("选 择 要 删 除 的 设 备 类 型");
        print_divider();
        print_option(0, "车床");
        print_option(1, "铣床");
        print_option(2, "钳工台");
        print_option(3, "焊机");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &dtype);
    } while (dtype < 0 || dtype > 3);

    int found = -1;
    for (int i = 0; i < safetyCount; i++) {
        if (strcmp(safetyDB[i].username, username) == 0 && safetyDB[i].device_type == dtype)
            { found = i; break; }
    }
    if (found < 0) { print_error("未找到记录。"); return; }
    for (int i = found; i < safetyCount - 1; i++) safetyDB[i] = safetyDB[i+1];
    safetyCount--;
    save_safety();
    print_success("资格/记录已删除。");
}

int user_safety_passed(const char *username, int device_type) {
    for (int i = 0; i < safetyCount; i++)
        if (strcmp(safetyDB[i].username, username) == 0 &&
            safetyDB[i].device_type == device_type && safetyDB[i].passed) return 1;
    return 0;
}

/* ========== PDF 导出（仅合格名单） ========== */
void export_safety_pass_list(void) {
    int passed_list[MAX_SAFETY];
    int pcount = 0;

    for (int i = 0; i < safetyCount; i++) {
        if (safetyDB[i].passed) passed_list[pcount++] = i;
    }

    if (pcount == 0) {
        print_warn("无已合格的安全考试记录，无法生成报表。");
        return;
    }

    char texPath[MAX_PATH_LEN];
    char pdfPath[MAX_PATH_LEN];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    char texName[64], pdfName[64];
    sprintf(texName, "safety_pass_%04d%02d%02d.tex", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    sprintf(pdfName, "safety_pass_%04d%02d%02d.pdf", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    get_output_file(texPath, texName);
    get_output_file(pdfPath, pdfName);

    FILE *fp = fopen(texPath, "wb");
    if (!fp) {
    print_error("无法创建安全资格报表文件，请检查是否使用了计算机管理员权限运行程序！");
        return;
    }

    U8F(fp, "\\documentclass{ctexart}\n");
    U8F(fp, "\\usepackage{geometry}\n");
    U8F(fp, "\\usepackage{array}\n");
    U8F(fp, "\\usepackage{booktabs}\n");
    U8F(fp, "\\usepackage{xcolor}\n");
    U8F(fp, "\\geometry{a4paper, margin=2.5cm}\n");
    U8F(fp, "\\begin{document}\n");
    U8F(fp, "\\thispagestyle{empty}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "{\\LARGE\\bfseries 安全资格考试合格名单}\\\\[0.3cm]\n");

    char info[256];
    sprintf(info, "{\\large 统计日期：%04d-%02d-%02d | 合格人次：%d}\\\\[0.5cm]\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, pcount);
    U8F(fp, info);
    U8F(fp, "\\end{center}\n");

    U8F(fp, "\\renewcommand{\\arraystretch}{1.2}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "\\begin{tabular}{|>{\\centering\\arraybackslash}p{2cm}|>{\\centering\\arraybackslash}p{3cm}|>{\\centering\\arraybackslash}p{3cm}|>{\\centering\\arraybackslash}p{3cm}|}\n");
    U8F(fp, "\\hline\n");
    U8F(fp, "\\textbf{序号} & \\textbf{用户名} & \\textbf{设备类型} & \\textbf{考试日期} \\\\\n");
    U8F(fp, "\\hline\n");

    for (int i = 0; i < pcount; i++) {
        SafetyExam *s = &safetyDB[passed_list[i]];
        char *u8_user = gbk_to_utf8(s->username);
        char *u8_type = gbk_to_utf8(device_type_name(s->device_type));
        char *u8_date = gbk_to_utf8(s->exam_date);

        fprintf(fp, "%d & %s & %s & %s \\\\\n",
                i + 1,
                u8_user ? u8_user : s->username,
                u8_type ? u8_type : device_type_name(s->device_type),
                u8_date ? u8_date : s->exam_date);
        U8F(fp, "\\hline\n");

        free(u8_user); free(u8_type); free(u8_date);
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
        print_success("安全资格合格名单PDF已生成：");
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