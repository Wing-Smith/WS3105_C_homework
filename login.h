#ifndef LOGIN_H
#define LOGIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <time.h>

#define MAX_USERS       100
#define MAX_DEVICES     50
#define MAX_RESERVE     200
#define MAX_MAINT       100
#define MAX_SAFETY      100
#define MAX_REPAIR      100
#define MAX_WORK        500
#define MAX_LOG         500
#define MAX_LINE        512
#define MAX_PATH_LEN    260

#define THEME_WELCOME   0x6F  
#define THEME_LOGIN     0xC9  
#define THEME_ADMIN     0x4F  
#define THEME_TEACHER   0x6F  
#define THEME_STUDENT   0x3F  
#define THEME_DEFAULT   0x0F  
#define THEME_ERROR     0xCF  
#define THEME_SUCCESS   0x2F  
#define THEME_WARN      0xE0  

typedef struct {
    char username[50];
    char password[50];
    int type;
} User;

typedef struct {
    char username[50];
    char password[50];
    int type;
    int status;
} PendingUser;

typedef struct {
    int  id;
    char name[50];
    int  type;
    char spec[50];
    char range[50];
    char precision[20];
    double total_hours;
    int  use_count;
    char status[20];
    int  danger;
    char next_maintenance[20];
} Device;

typedef struct {
    int  id;
    int  device_id;
    char username[50];
    int  user_type;
    char date[20];
    char time_slot[20];
    char purpose[30];
    char priority[10];
    char status[20];
    char create_time[30];
    char checkin_time[30];
} Reservation;

typedef struct {
    int  id;
    int  device_id;
    char date[20];
    char content[100];
    char next_date[20];
} Maintenance;

typedef struct {
    char username[50];
    int  device_type;
    int  passed;
    char exam_date[20];
} SafetyExam;

typedef struct {
    int  id;
    int  device_id;
    char reporter[50];
    char date[20];
    char description[100];
    char status[20];
    char assignee[50];
    char complete_date[20];
} RepairOrder;

typedef struct {
    char username[50];
    int  device_id;
    char date[20];
    double duration;
    int  score;
    char comment[100];
} WorkHour;

typedef struct {
    char timestamp[30];
    char username[50];
    char action[50];
    char detail[200];
} OpLog;

extern User         userDB[MAX_USERS];
extern int          userCount;
extern int          currentType;
extern char         currentUser[50];

extern PendingUser  pendingDB[MAX_USERS];
extern int          pendingCount;

extern Device       deviceDB[MAX_DEVICES];
extern int          deviceCount;

extern Reservation  reserveDB[MAX_RESERVE];
extern int          reserveCount;

extern Maintenance  maintDB[MAX_MAINT];
extern int          maintCount;

extern SafetyExam   safetyDB[MAX_SAFETY];
extern int          safetyCount;

extern RepairOrder  repairDB[MAX_REPAIR];
extern int          repairCount;

extern WorkHour     workDB[MAX_WORK];
extern int          workCount;

extern OpLog        logDB[MAX_LOG];
extern int          logCount;

extern char dataDir[MAX_PATH_LEN];
extern int current_theme;

/* ≈≈∞Ê”Î—’…´ */
void set_color(int color);
void apply_theme(int color);
void restore_theme(void);
void print_centered(const char *text);
void print_divider(void);
void print_option(int num, const char *text);
void print_prompt(const char *text);
void print_error(const char *text);
void print_success(const char *text);
void print_warn(const char *text);
int  get_console_width(void);

void clear_screen(void);
void pause_screen(void);
void trim_newline(char *str);
void get_datetime(char *buf, int len);
int  parse_line_user(char *line, User *u);
int  parse_line_pending(char *line, PendingUser *p);

void getAppRelativePath(void);
void ensure_dir(void);
FILE* open_csv(const char *name, const char *mode);
void get_output_dir(char *buf);
void get_output_file(char *buf, const char *filename);
void ensure_data_exists(void);
void ensure_pending_exists(void);
void ensure_all_csv(void);

int load_users(void);
int save_users(void);
int load_pending_users(void);
int save_pending_users(void);
int do_login_check(const char *username, const char *password);
int check_pending_status(const char *username, const char *password);
int register_user(const char *username, const char *password, int type);
int user_exists(const char *username);
int pending_username_exists(const char *username);
const char* type_name(int type);

const char* device_type_name(int type);
const char* danger_name(int d);

int load_devices(void);
int save_devices(void);
void list_devices(void);
void add_device(void);
void delete_device(void);
void device_maintenance_warn(void);

int load_reservations(void);
int save_reservations(void);
void list_my_reservations(const char *username);
int add_reservation(const char *username, int user_type);
void cancel_reservation(const char *username);
void checkin_reservation(const char *username);
int check_time_conflict(int device_id, const char *date, const char *time_slot, int exclude_id);
int check_safety_pass(const char *username, int device_type);
void auto_release_overdue(void);

int load_maintenance(void);
int save_maintenance(void);
void list_maintenance(void);
void add_maintenance(void);

int load_safety(void);
int save_safety(void);
void list_safety(void);
void add_safety_record(void);
void remove_safety_record(void);
void export_safety_pass_list(void);
int user_safety_passed(const char *username, int device_type);

int load_repairs(void);
int save_repairs(void);
void list_repairs(void);
void export_repair_report(void);
void report_repair(void);
void assign_repair(void);
void complete_repair(void);

int load_work_hours(void);
int save_work_hours(void);
void list_work_hours(void);
void add_work_hour(void);
void export_work_hours(void);

int load_logs(void);
int save_logs(void);
void list_logs(void);
void add_log(const char *username, const char *action, const char *detail);

void device_utilization_report(void);

void export_device_report(void);

void generate_certificate(const char *username);

#endif