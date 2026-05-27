#include "login.h"
#include "teacher.h"

int teacher_menu(void) {
    int choice;
    int confirm;
    do {
        apply_theme(THEME_TEACHER);
        print_divider();
        print_centered("教 师 菜 单");
        print_divider();
        char buf[128];
        sprintf(buf, "当前用户: %s [%s]", currentUser, type_name(currentType));
        print_centered(buf);
        printf("\n");
        print_option(1, "查询学生信息");
        print_option(2, "录入成绩/工时");
        print_option(3, "班级/个人工时与成绩统计导出");
        print_option(4, "设备利用率报表");
        print_option(5, "故障报修");
        print_option(6, "查看报修进度");
        print_option(7, "安全考试名单导入");
        print_option(8, "安全资格剥夺");
        print_option(9, "设备预约");
        print_option(10, "我的预约");
        print_option(11, "取消/改期预约");
        print_option(12, "签到");
        print_option(0, "退出账户（返回欢迎界面）");
        print_option(99, "退出系统");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                apply_theme(THEME_TEACHER);
                print_divider();
                print_centered("查 询 学 生 信 息");
                print_divider();
                print_centered("当前学生用户列表:");
                for (int i = 0; i < userCount; i++) {
                    if (userDB[i].type == 3) {
                        char line[64];
                        sprintf(line, "  %-20s", userDB[i].username);
                        print_centered(line);
                    }
                }
                pause_screen();
                break;
            case 2:
                apply_theme(THEME_TEACHER);
                add_work_hour();
                pause_screen();
                break;
            case 3:
                apply_theme(THEME_TEACHER);
                export_work_hours();
                pause_screen();
                break;
            case 4:
                apply_theme(THEME_TEACHER);
                device_utilization_report();
                printf("\n");
                print_prompt("是否导出PDF报表? (1=确认, 0=取消): ");
                scanf("%d", &confirm);
                if (confirm == 1) export_device_report();
                pause_screen();
                break;
            case 5:
                apply_theme(THEME_TEACHER);
                report_repair();
                pause_screen();
                break;
            case 6:
                apply_theme(THEME_TEACHER);
                print_divider();
                print_centered("报 修 进 度 查 看");
                print_divider();
                list_repairs();
                printf("\n");
                print_prompt("是否导出PDF报表? (1=确认, 0=取消): ");
                scanf("%d", &confirm);
                if (confirm == 1) export_repair_report();
                pause_screen();
                break;
            case 7:
                apply_theme(THEME_TEACHER);
                add_safety_record();
                pause_screen();
                break;
            case 8: {
                int sub;
                do {
                    apply_theme(THEME_TEACHER);
                    print_divider();
                    print_centered("安 全 资 格 管 理");
                    print_divider();
                    print_option(1, "查看合格记录");
                    print_option(2, "新增考试记录");
                    print_option(3, "删除资格记录");
                    print_option(4, "导出合格名单PDF");
                    print_option(0, "返回");
                    printf("\n");
                    print_prompt("请选择: ");
                    scanf("%d", &sub);
                    switch (sub) {
                        case 1: apply_theme(THEME_TEACHER); print_divider(); print_centered("安 全 考 试 合 格 记 录"); print_divider(); list_safety(); pause_screen(); break;
                        case 2: apply_theme(THEME_TEACHER); add_safety_record(); pause_screen(); break;
                        case 3: apply_theme(THEME_TEACHER); remove_safety_record(); pause_screen(); break;
                        case 4: apply_theme(THEME_TEACHER); export_safety_pass_list(); pause_screen(); break;
                    }
                } while (sub != 0);
                break;
            }
            case 9:
                apply_theme(THEME_TEACHER);
                add_reservation(currentUser, currentType);
                pause_screen();
                break;
            case 10:
                apply_theme(THEME_TEACHER);
                list_my_reservations(currentUser);
                pause_screen();
                break;
            case 11:
                apply_theme(THEME_TEACHER);
                cancel_reservation(currentUser);
                pause_screen();
                break;
            case 12:
                apply_theme(THEME_TEACHER);
                checkin_reservation(currentUser);
                pause_screen();
                break;
            case 0:
                apply_theme(THEME_TEACHER);
                print_centered("已退出账户，返回欢迎界面。");
                add_log(currentUser, "退出账户", "返回欢迎界面");
                return 0;
            case 99:
                apply_theme(THEME_TEACHER);
                print_centered("感谢使用，再见！");
                return 1;
            default:
                print_error("无效选择。");
                pause_screen();
        }
    } while (1);
}