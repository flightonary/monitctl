#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define NO_CHANGE -1

#define BUF_SIZE 256
#define MAX_ACTION 32

#ifndef CONFIG_FILE
    #define CONFIG_FILE "/etc/monitctl.conf"
#endif

typedef struct {
    char *config_file;
    char *group_name;
    char *action;
    char *target;
} Options;

typedef struct {
    char *monit_path;
    char **allow_actions;
    int  action_num;
} Configs;

void print_usege(void)
{
    fprintf(stdout, "Usage: monitctl [OPTION]... MONIT_ACTION\n"
                    " monitctl calls optional action arguments of monit.\n"
                    " Options are as follows:\n"
                    "  -g name          Set group name for start, stop, restart, monitor and unmonitor\n"
                    "  -c config_file   Use this monitctl control file (default:" CONFIG_FILE ")\n"
                    );
}

Options* get_options(int argc, char *argv[])
{
    Options *opts;
    int opt;

    opts = (Options *)malloc(sizeof(Options));
    opts->config_file = CONFIG_FILE;
    opts->group_name  = NULL;
    opts->action      = NULL;
    opts->target      = NULL;

    while ((opt = getopt(argc, argv, "c:g:")) != -1) {
        switch (opt) {
            case 'g':
            opts->group_name = optarg;
            break;
            case 'c':
            opts->config_file = optarg;
            break;
        default: /* '?' or ':' */
            return NULL;
        }
    }

    if(optind >= argc) {
        return NULL;
    }

    opts->action = argv[optind];
    if(optind + 1 < argc) {
        opts->target = argv[optind + 1];
    }

    return opts;
}

bool check_config_file_permission(Options *opts)
{
    struct stat info;

    if(access(opts->config_file, F_OK) != 0) {
        fprintf(stderr, "[monitctl] config file(%s) does not exist\n", opts->config_file);
        return false;
    }

    if(stat(opts->config_file, &info) != 0) {
        fprintf(stderr, "[monitctl] Error in getting config file's stat\n");
        return false;
    }

    if(S_ISREG(info.st_mode) &&                          // Regular file and,
       (info.st_mode & (S_IWGRP | S_IWOTH)) == 0x00 &&   // Group and other are not allowed to write and,
       info.st_uid == geteuid()                          // file owner and effective uid is same.
       ) {
        return true;
    }

    fprintf(stderr, "[monitctl] Group and other are not allowed to write config file(%s) and "
                    "owner must be same as effective uid\n", opts->config_file);
    return false;
}

Configs* get_configs(Options *opts)
{
    Configs *configs;
    FILE *fp;
    char line[BUF_SIZE], key[BUF_SIZE], value[BUF_SIZE];
    char *action;
    int index;

    configs = (Configs *)malloc(sizeof(Configs));
    configs->monit_path = "";
    configs->allow_actions = (char **)malloc(sizeof(char *) * MAX_ACTION);
    configs->action_num = 0;

    fp = fopen(opts->config_file, "r");
    if(fp == NULL) {
        fprintf(stderr, "[monitctl] Error in openning config file (file:%s)\n", opts->config_file);
        return NULL;
    }

    while(fgets(line, sizeof(line), fp) != NULL) {
        if(line[0] == '#') {
            continue;
        }

        sscanf(line, "%255s%255s", key, value);
        if(strcmp(key, "monit_path") == 0) {
            configs->monit_path = strdup(value);
        }
        else if(strcmp(key, "allow_actions") == 0) {
            action = strtok(value, ",");
            while(action && configs->action_num < MAX_ACTION) {
                index = configs->action_num;
                configs->allow_actions[index] = strdup(action);
                configs->action_num++;

                action = strtok(NULL, ",");
            }
        }
    }

#ifdef DEBUG
    fprintf(stderr, "[DEBUG] monit_path=%s\n", configs->monit_path);
    for(int i = 0; i < configs->action_num; i++) {
        fprintf(stderr, "[DEBUG] allow_action%02d=%s\n", i, configs->allow_actions[i]);
    }
#endif

    return configs;
}

bool check_allow_actions(Options *opts, Configs *configs)
{
    int i;

    for(i = 0; i < configs->action_num; i++) {
        if(strcmp(opts->action, configs->allow_actions[i]) == 0)
            return true;
    }

    fprintf(stderr, "[monitctl] Action '%s' is forbidden\n", opts->action);

    return false;
}

bool reset_real_uid(void)
{
    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;
    int ret_getresuid, ret_getresgid;
    int ret_setreuid, ret_setregid;

    // get effective uid and effective gid.
    ret_getresuid = getresuid(&ruid, &euid, &suid);
    ret_getresgid = getresgid(&rgid, &egid, &sgid);

    if(ret_getresuid != 0 || ret_getresgid != 0) {
        fprintf(stderr, "[monitctl] Error: getresuid/getresgid\n");
        return false;
    }

    // set effective uid to real uid.
    // set effective gid to real gid.
    ret_setreuid = setreuid(euid, NO_CHANGE);
    ret_setregid = setregid(egid, NO_CHANGE);

    if(ret_setreuid != 0 || ret_setregid != 0) {
        fprintf(stderr, "[monitctl] Error: setreuid/setregid\n");
        return false;
    }

    return true;
}

void invoke_monit(Options *opts, Configs *configs)
{
    char *args[5];
    int index = 0;

    args[index++] = configs->monit_path;
    if(opts->group_name) {
        args[index++] = "-g";
        args[index++] = opts->group_name;
    }
    args[index++] = opts->action;
    if(opts->target) {
        args[index++] = opts->target;
    }
    args[index++] = NULL;

    execv(configs->monit_path, args);
}

int main(int argc, char *argv[])
{
    Options *opts;
    Configs *configs;

    opts = get_options(argc, argv);
    if(opts == NULL) {
        print_usege();
        exit(EXIT_FAILURE);
    }

    if(reset_real_uid() == false) {
        exit(EXIT_FAILURE);
    }

    if(check_config_file_permission(opts) == false) {
        exit(EXIT_FAILURE);
    }

    configs = get_configs(opts);
    if(configs == NULL) {
        exit(EXIT_FAILURE);
    }

    if(check_allow_actions(opts, configs) == false) {
        exit(EXIT_FAILURE);
    }

    invoke_monit(opts, configs);

    fprintf(stderr, "[monitctl] Error in executing monit\n");
    exit(EXIT_FAILURE);
}
