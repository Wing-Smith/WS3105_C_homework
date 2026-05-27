#include "login.h"
#include <windows.h>

/* GBK 转 UTF-8，返回 malloc 的字符串，调用者负责 free */
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

/* 辅助宏：把 GBK 字符串转为 UTF-8 写入文件，然后立即释放 */
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

void generate_certificate(const char *username) {
    double total_hours = 0;
    int count = 0, total_score = 0;

    for (int i = 0; i < workCount; i++) {
        if (strcmp(workDB[i].username, username) == 0) {
            count++;
            total_hours += workDB[i].duration;
            total_score += workDB[i].score;
        }
    }

    if (count == 0) {
        print_warn("该学员暂无实习记录，无法生成证书。");
        return;
    }

    char texPath[MAX_PATH_LEN];
    get_output_file(texPath, "certificate.tex");

    FILE *fp = fopen(texPath, "wb");
    if (!fp) {
        print_error("无法创建证明文件，请检查程序是否以计算机管理员身份运行");
        return;
    }

    /* ===== LaTeX 头部 ===== */
    U8F(fp, "\\documentclass{ctexart}\n");
    U8F(fp, "\\usepackage{geometry}\n");
    U8F(fp, "\\usepackage{array}\n");
    U8F(fp, "\\usepackage{booktabs}\n");
    U8F(fp, "\\usepackage{xcolor}\n");
    U8F(fp, "\\geometry{a4paper, margin=2.5cm}\n");
    U8F(fp, "\\begin{document}\n");
    U8F(fp, "\\thispagestyle{empty}\n");
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "{\\LARGE\\bfseries 昆明理工大学}\\\\[0.3cm]\n");
    U8F(fp, "{\\Large\\bfseries 金工实习证书}\\\\[0.5cm]\n");
    U8F(fp, "{\\small\\textcolor{red}{注意：未加盖教务处或实训中心公章前，本证书无效}}\n");
    U8F(fp, "\\end{center}\n");
    U8F(fp, "\\vspace{1cm}\n");

    /* 姓名行 */
    char *u8_name = gbk_to_utf8(username);
    U8F(fp, "\\noindent 兹证明 \\underline{\\textbf{");
    if (u8_name) { fputs(u8_name, fp); free(u8_name); }
    U8F(fp, "}} 同学完成本校金工实习课程，具体实习内容如下：\n");
    U8F(fp, "\\vspace{0.5cm}\n\n");

    /* 表格 */
    U8F(fp, "\\begin{center}\n");
    U8F(fp, "\\renewcommand{\\arraystretch}{1.3}\n");
    U8F(fp, "\\begin{tabular}{|c|c|c|c|p{6cm}|}\n");
    U8F(fp, "\\hline\n");
    U8F(fp, "\\textbf{设备ID} & \\textbf{实习日期} & \\textbf{工时(h)} & \\textbf{成绩} & \\textbf{评语} \\\\\n");
    U8F(fp, "\\hline\n");

    for (int i = 0; i < workCount; i++) {
        if (strcmp(workDB[i].username, username) == 0) {
            char esc_comment[256];
            latex_escape(esc_comment, workDB[i].comment, sizeof(esc_comment));
            char *u8_comment = gbk_to_utf8(esc_comment);
            char *u8_date    = gbk_to_utf8(workDB[i].date);
            fprintf(fp, "%d & %s & %.1f & %d & %s \\\\\n",
                    workDB[i].device_id,
                    u8_date ? u8_date : workDB[i].date,
                    workDB[i].duration,
                    workDB[i].score,
                    u8_comment ? u8_comment : esc_comment);
            U8F(fp, "\\hline\n");
            free(u8_comment);
            free(u8_date);
        }
    }

    U8F(fp, "\\end{tabular}\n");
    U8F(fp, "\\end{center}\n");
    U8F(fp, "\\vspace{0.8cm}\n\n");
    U8F(fp, "\\noindent\n");
    U8F(fp, "\\begin{tabular}{ll}\n");

    char buf[128];
    sprintf(buf, "\\textbf{总实习工时} & %.1f 小时 \\\\\n", total_hours);
    U8F(fp, buf);
    sprintf(buf, "\\textbf{平均成绩} & %.1f 分 \\\\\n", count > 0 ? (double)total_score / count : 0);
    U8F(fp, buf);
    sprintf(buf, "\\textbf{实习记录条数} & %d 条 \\\\\n", count);
    U8F(fp, buf);

    U8F(fp, "\\end{tabular}\n");
    U8F(fp, "\\vspace{1.5cm}\n\n");
    U8F(fp, "\\begin{flushright}\n");
    U8F(fp, "\\textbf{昆明理工大学实训中心}\\\\[0.3cm]\n");

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    sprintf(buf, "%04d年%02d月%02d日\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    U8F(fp, buf);

    U8F(fp, "\\end{flushright}\n");
    U8F(fp, "\\end{document}\n");

    fclose(fp);

    char cmd[512];
    char outDir[MAX_PATH_LEN];
    get_output_dir(outDir);
    sprintf(cmd, "pdflatex -interaction=nonstopmode -output-directory=\"%s\" \"%s\" > nul 2>&1", outDir, texPath);
    int ret = system(cmd);

    char pdfPath[MAX_PATH_LEN];
    get_output_file(pdfPath, "certificate.pdf");

    if (ret == 0) {
        print_success("实习证书PDF已生成：");
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