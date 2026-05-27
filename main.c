#include "login.h"
#include "admin.h"
#include "teacher.h"
#include "student.h"

void welcome_screen(void) {
    apply_theme(THEME_WELCOME);
    print_divider();
    print_centered("欢 迎 使 用 金 工 实 习 管 理 系 统");
    print_centered("          ");
    print_centered("(C)Wing North Studio 2026");
    print_centered("Built By WS3105（王子文） & 王俊涛");
    print_centered("本程序仅供展示C语言程序设计思想，严禁用于一切商业用途！");
    print_centered("GitHub 开源仓库地址：https://github.com/Wing-Smith/WS3105_C_homework");
    print_divider();
    printf("\n");
    print_option(1, "登录");
    print_option(2, "注册");
    print_option(0, "退出系统");
    printf("\n");
    print_prompt("请选择: ");
}

int do_login(void) {
    char username[50], password[50];
    int attempts = 3;

    apply_theme(THEME_LOGIN);
    print_divider();
    print_centered("用 户 登 录");
    print_divider();
    printf("\n");

    while (attempts > 0) {
        print_prompt("用户名: ");
        scanf("%49s", username);
        print_prompt("密码: ");
        scanf("%49s", password);

        if (do_login_check(username, password)) {
            apply_theme(THEME_LOGIN);
            print_success("登录成功！");
            char buf[128];
            sprintf(buf, "欢迎, %s [%s]", currentUser, type_name(currentType));
            print_centered(buf);
            printf("\n");
            add_log(currentUser, "登录", "用户登录系统");
            return 1;
        }

        int pstatus = check_pending_status(username, password);
        if (pstatus == 0) {
            print_warn("警告：该账户仍在审核中，请等待管理员审批。");
            pause_screen();
            return 0;
        } else if (pstatus == 1) {
            print_warn("警告：该注册申请已被管理员驳回，无法登录。");
            pause_screen();
            return 0;
        }

        attempts--;
        char buf[128];
        sprintf(buf, "登录失败，剩余次数: %d", attempts);
        print_error(buf);
        printf("\n");
    }

    print_error("次数用尽，返回欢迎界面。");
    pause_screen();
    return 0;
}

int do_register(void) {
    char username[50], password[50];
    int type;

    apply_theme(THEME_LOGIN);
    print_divider();
    print_centered("用 户 注 册");
    print_divider();
    printf("\n");

    print_prompt("用户名: ");
    scanf("%49s", username);

    if (user_exists(username)) {
        print_error("错误：该用户名已被注册。");
        pause_screen();
        return 0;
    }

    if (pending_username_exists(username)) {
        print_error("错误：该用户名已在审核中，请勿重复注册。");
        pause_screen();
        return 0;
    }

    print_prompt("密码: ");
    scanf("%49s", password);

    do {
        apply_theme(THEME_LOGIN);
        print_divider();
        print_centered("选 择 账 户 类 型");
        print_divider();
        print_option(2, "教师");
        print_option(3, "学生");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &type);
    } while (type != 2 && type != 3);

    if (register_user(username, password, type)) {
        print_success("注册申请已提交，等待管理员审批。");
    } else {
        print_error("注册失败。");
    }
    pause_screen();
    return 1;
}

int show_menu(void) {
    switch (currentType) {
        case 1: return admin_menu();
        case 2: return teacher_menu();
        case 3: return student_menu();
        default:
            print_error("未知用户类型。");
            return 0;
    }
}

int main(void) {
    int choice;

    getAppRelativePath();
    ensure_data_exists();
    ensure_pending_exists();
    ensure_all_csv();

    if (!load_users() || !load_pending_users() || !load_devices() ||
        !load_reservations() || !load_maintenance() || !load_safety() ||
        !load_repairs() || !load_work_hours() || !load_logs()) {
        apply_theme(THEME_DEFAULT);
        print_error("数据加载失败，程序退出。");
        return 1;
    }

    while (1) {
        welcome_screen();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (do_login()) {
                    int ret = show_menu();
                    if (ret == 1) {
                        apply_theme(THEME_WELCOME);
                        print_centered("感谢使用，再见！");
                        return 0;
                    }
                }
                break;
            case 2:
                do_register();
                break;
            case 0:
                apply_theme(THEME_WELCOME);
                print_centered("感谢使用，再见！");
                return 0;
            default:
                print_error("无效选择。");
                pause_screen();
        }
    }
}