#include "login.h"
#include "student.h"

int student_menu(void) {
    int choice;
    do {
        apply_theme(THEME_STUDENT);
        print_divider();
        print_centered("学 生 菜 单");
        print_divider();
        char buf[128];
        sprintf(buf, "当前用户: %s [%s]", currentUser, type_name(currentType));
        print_centered(buf);
        printf("\n");
        print_option(1, "查询成绩/工时");
        print_option(2, "设备预约");
        print_option(3, "我的预约");
        print_option(4, "取消/改期预约");
        print_option(5, "签到");
        print_option(6, "证明开具");
        print_option(7, "个人信息");
        print_option(0, "退出账户（返回欢迎界面）");
        print_option(99, "退出系统");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                apply_theme(THEME_STUDENT);
                print_divider();
                print_centered("我 的 实 习 工 时 与 成 绩");
                print_divider();
                printf("\n");
                printf("  %-8s %-12s %-8s %-6s %-20s\n", "设备ID","日期","工时","成绩","评语");
                print_divider();
                for (int i = 0; i < workCount; i++) {
                    if (strcmp(workDB[i].username, currentUser) == 0) {
                        WorkHour *w = &workDB[i];
                        printf("  %-8d %-12s %-8.1f %-6d %-20s\n",
                               w->device_id, w->date, w->duration, w->score, w->comment);
                    }
                }
                pause_screen();
                break;
            }
            case 2:
                apply_theme(THEME_STUDENT);
                add_reservation(currentUser, currentType);
                pause_screen();
                break;
            case 3:
                apply_theme(THEME_STUDENT);
                list_my_reservations(currentUser);
                pause_screen();
                break;
            case 4:
                apply_theme(THEME_STUDENT);
                cancel_reservation(currentUser);
                pause_screen();
                break;
            case 5:
                apply_theme(THEME_STUDENT);
                checkin_reservation(currentUser);
                pause_screen();
                break;
            //旧版本CASE6: 直接显示实习证明信息
            /*case 6:
                apply_theme(THEME_STUDENT);
                print_divider();
                print_centered("实 习 证 明 开 具");
                print_divider();
                printf("\n");
                char info[128];
                sprintf(info, "姓名: %s", currentUser);
                print_centered(info);
                print_centered("实习记录:");
                for (int i = 0; i < workCount; i++) {
                    if (strcmp(workDB[i].username, currentUser) == 0) {
                        char line[128];
                        sprintf(line, "设备ID:%d 日期:%s 工时:%.1fh 成绩:%d",
                               workDB[i].device_id, workDB[i].date, workDB[i].duration, workDB[i].score);
                        print_centered(line);
                    }
                }
                printf("\n");
                print_centered("（以上信息可用于开具实习证明）");
                pause_screen();
                break;*/
            //新版本CASE6: 调用函数生成实习证明
            case 6:
                apply_theme(THEME_STUDENT);
                print_divider();
                print_centered("实 习 证 明 开 具");
                print_divider();
                printf("\n");
                generate_certificate(currentUser);
                pause_screen();
                break;
            case 7:
                apply_theme(THEME_STUDENT);
                print_divider();
                print_centered("个 人 信 息");
                print_divider();
                printf("\n");
                char uinfo[128];
                sprintf(uinfo, "用户名: %s", currentUser);
                print_centered(uinfo);
                sprintf(uinfo, "用户类型: %s", type_name(currentType));
                print_centered(uinfo);
                printf("\n");
                print_centered("安全考试合格情况:");
                for (int i = 0; i < safetyCount; i++) {
                    if (strcmp(safetyDB[i].username, currentUser) == 0) {
                        char line[128];
                        sprintf(line, "%-12s: %s", device_type_name(safetyDB[i].device_type),
                               safetyDB[i].passed ? "合格" : "不合格");
                        print_centered(line);
                    }
                }
                pause_screen();
                break;
            case 0:
                apply_theme(THEME_STUDENT);
                print_centered("已退出账户，返回欢迎界面。");
                add_log(currentUser, "退出账户", "返回欢迎界面");
                return 0;
            case 99:
                apply_theme(THEME_STUDENT);
                print_centered("感谢使用，再见！");
                return 1;
            default:
                print_error("无效选择。");
                pause_screen();
        }
    } while (1);
}