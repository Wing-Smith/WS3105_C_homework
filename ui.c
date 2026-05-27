#include "login.h"

int current_theme = THEME_DEFAULT;

/* ========== 颜色与排版 ========== */
void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

int get_console_width(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.dwSize.X;
    return 80;
}

void apply_theme(int color) {
    current_theme = color;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    COORD home = {0, 0};
    set_color(color);
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hOut, ' ', cells, home, &count);
        FillConsoleOutputAttribute(hOut, color, cells, home, &count);
        SetConsoleCursorPosition(hOut, home);
    } else {
        system("cls");
    }
}

void restore_theme(void) {
    set_color(current_theme);
}

void clear_screen(void) {
    apply_theme(current_theme);
}

void print_centered(const char *text) {
    int w = get_console_width();
    int len = strlen(text);
    int pad = (w - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s", text);
    int after = w - pad - len;
    for (int i = 0; i < after; i++) printf(" ");
    printf("\n");
}

void print_divider(void) {
    int w = get_console_width();
    for (int i = 0; i < w; i++) printf("=");
    printf("\n");
}

void print_option(int num, const char *text) {
    char buf[256];
    sprintf(buf, "[%d] %s", num, text);
    print_centered(buf);
}

void print_prompt(const char *text) {
    int w = get_console_width();
    int len = strlen(text);
    int pad = (w - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s", text);
}

void print_error(const char *text) {
    set_color(THEME_ERROR);
    print_centered(text);
    restore_theme();
}

void print_success(const char *text) {
    set_color(THEME_SUCCESS);
    print_centered(text);
    restore_theme();
}

void print_warn(const char *text) {
    set_color(THEME_WARN);
    print_centered(text);
    restore_theme();
}

void pause_screen(void) {
    printf("\n");
    print_centered("按 Enter 键继续...");
    getchar();
    getchar();
}