#include "login.h"
#include "admin.h"

int admin_menu(void) {
    int choice;
    int confirm;
    do {
        apply_theme(THEME_ADMIN);
        print_divider();
        print_centered("管 理 员 菜 单");
        print_divider();
        char buf[128];
        sprintf(buf, "当前用户: %s [%s]", currentUser, type_name(currentType));
        print_centered(buf);
        printf("\n");
        print_option(1, "查看所有用户");
        print_option(2, "系统设置");
        print_option(3, "数据备份");
        print_option(4, "注册申请审批");
        print_option(5, "设备信息管理");
        print_option(6, "检修记录与保养预警");
        print_option(7, "安全考试名单管理");
        print_option(8, "故障报修闭环处理");
        print_option(9, "操作日志审计");
        print_option(10, "设备利用率报表");
        print_option(0, "退出账户（返回欢迎界面）");
        print_option(99, "退出系统");
        printf("\n");
        print_prompt("请选择: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                apply_theme(THEME_ADMIN);
                print_divider();
                print_centered("当 前 系 统 用 户");
                print_divider();
                printf("\n");
                for (int i = 0; i < userCount; i++) {
                    char line[128];
                    sprintf(line, "  %-20s [%s]", userDB[i].username, type_name(userDB[i].type));
                    print_centered(line);
                }
                pause_screen();
                break;
            case 2:
                apply_theme(THEME_ADMIN);
                print_divider();
                print_centered("系 统 设 置");
                print_divider();
                print_centered("进入系统设置...");
                pause_screen();
                break;
            case 3:
                apply_theme(THEME_ADMIN);
                print_divider();
                print_centered("数 据 备 份");
                print_divider();
                print_success("数据备份完成。");
                pause_screen();
                break;
            case 4: {
                apply_theme(THEME_ADMIN);
                print_divider();
                print_centered("注 册 申 请 审 批");
                print_divider();
                printf("\n");

                int list[MAX_USERS];
                int count = 0;
                for (int i = 0; i < pendingCount; i++) {
                    if (pendingDB[i].status == 0) {
                        char line[128];
                        sprintf(line, "%d. %-15s [%s]", count + 1, pendingDB[i].username, type_name(pendingDB[i].type));
                        print_centered(line);
                        list[count] = i;
                        count++;
                    }
                }

                if (count == 0) {
                    print_centered("暂无待审批的申请。");
                    pause_screen();
                    break;
                }

                printf("\n");
                print_prompt("请选择申请编号 (0=返回): ");
                int idx;
                scanf("%d", &idx);
                if (idx <= 0 || idx > count) break;

                int pidx = list[idx - 1];
                apply_theme(THEME_ADMIN);
                char info[256];
                sprintf(info, "用户名: %s  类型: %s", pendingDB[pidx].username, type_name(pendingDB[pidx].type));
                print_centered(info);
                printf("\n");
                print_option(1, "通过审批");
                print_option(2, "驳回申请");
                print_option(0, "取消");
                printf("\n");
                print_prompt("请选择: ");
                int action;
                scanf("%d", &action);

                if (action == 1) {
                    strcpy(userDB[userCount].username, pendingDB[pidx].username);
                    strcpy(userDB[userCount].password, pendingDB[pidx].password);
                    userDB[userCount].type = pendingDB[pidx].type;
                    userCount++;
                    save_users();

                    pendingDB[pidx].status = 2;
                    save_pending_users();
                    print_success("审批通过，用户已加入系统。");
                    add_log(currentUser, "审批通过", pendingDB[pidx].username);
                } else if (action == 2) {
                    pendingDB[pidx].status = 1;
                    save_pending_users();
                    print_success("申请已驳回。");
                    add_log(currentUser, "审批驳回", pendingDB[pidx].username);
                }
                pause_screen();
                break;
            }
            case 5: {
                int sub;
                do {
                    apply_theme(THEME_ADMIN);
                    print_divider();
                    print_centered("设 备 信 息 管 理");
                    print_divider();
                    print_option(1, "查看设备列表");
                    print_option(2, "添加设备");
                    print_option(3, "删除设备");
                    print_option(0, "返回");
                    printf("\n");
                    print_prompt("请选择: ");
                    scanf("%d", &sub);
                    switch (sub) {
                        case 1: apply_theme(THEME_ADMIN); list_devices(); pause_screen(); break;
                        case 2: apply_theme(THEME_ADMIN); add_device(); pause_screen(); break;
                        case 3: apply_theme(THEME_ADMIN); delete_device(); pause_screen(); break;
                    }
                } while (sub != 0);
                break;
            }
            case 6:
                apply_theme(THEME_ADMIN);
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
            case 7: {
                int sub;
                do {
                    apply_theme(THEME_ADMIN);
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
                        case 1: apply_theme(THEME_ADMIN); print_divider(); print_centered("安 全 考 试 合 格 记 录"); print_divider(); list_safety(); pause_screen(); break;
                        case 2: apply_theme(THEME_ADMIN); add_safety_record(); pause_screen(); break;
                        case 3: apply_theme(THEME_ADMIN); remove_safety_record(); pause_screen(); break;
                        case 4: apply_theme(THEME_ADMIN); export_safety_pass_list(); pause_screen(); break;
                    }
                } while (sub != 0);
                break;
            }
            case 8: {
                int sub;
                do {
                    apply_theme(THEME_ADMIN);
                    print_divider();
                    print_centered("故 障 报 修 闭 环 处 理");
                    print_divider();
                    print_option(1, "查看报修单");
                    print_option(2, "派修");
                    print_option(3, "完工验收");
                    print_option(0, "返回");
                    printf("\n");
                    print_prompt("请选择: ");
                    scanf("%d", &sub);
                    switch (sub) {
                        case 1: apply_theme(THEME_ADMIN); print_divider(); print_centered("故 障 报 修 单"); print_divider(); list_repairs(); pause_screen(); break;
                        case 2: apply_theme(THEME_ADMIN); assign_repair(); pause_screen(); break;
                        case 3: apply_theme(THEME_ADMIN); complete_repair(); pause_screen(); break;
                    }
                } while (sub != 0);
                break;
            }
            case 9:
                apply_theme(THEME_ADMIN);
                print_divider();
                print_centered("操 作 日 志 审 计");
                print_divider();
                list_logs();
                pause_screen();
                break;
            case 10:
                apply_theme(THEME_ADMIN);
                device_utilization_report();
                printf("\n");
                print_prompt("是否导出PDF报表? (1=确认, 0=取消): ");
                scanf("%d", &confirm);
                if (confirm == 1) export_device_report();
                pause_screen();
                break;
            case 0:
                apply_theme(THEME_ADMIN);
                print_centered("已退出账户，返回欢迎界面。");
                add_log(currentUser, "退出账户", "返回欢迎界面");
                return 0;
            case 99:
                apply_theme(THEME_ADMIN);
                print_centered("感谢使用，再见！");
                return 1;
            default:
                print_error("无效选择。");
                pause_screen();
        }
    } while (1);
}