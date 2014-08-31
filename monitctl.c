#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define NO_CHANGE -1

#ifndef CONFIG_FILE
	#define CONFIG_FILE "/etc/monitctl"
#endif

typedef struct {
	char *config_file;
	char *group_name;
	char *action;
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
					"  -c config_file   Use this monitctl control file\n"
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

    return opts;
}

Configs* get_configs(Options *opts)
{
	Configs *configs;

	configs = (Configs *)malloc(sizeof(Configs));
	configs->monit_path = "/usr/bin/monit";

	return configs;
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
		fprintf(stderr, "getresuid/getresgid error\n");
		return false;
	}

	// set effective uid to real uid.
	// set effective gid to real gid.
	ret_setreuid = setreuid(euid, NO_CHANGE);
	ret_setregid = setregid(egid, NO_CHANGE);

	if(ret_setreuid != 0 || ret_setregid != 0) {
		fprintf(stderr, "setreuid/setregid error\n");
		return false;
	}

	return true;
}

void invoke_monit(Options *opts, Configs *configs)
{
	char *args[4];
	int index = 0;

	args[index++] = configs->monit_path;
	if(opts->group_name) {
		args[index++] = "-g";
		args[index++] = opts->group_name;
	}
	args[index++] = opts->action;
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

	configs = get_configs(opts);
	if(configs == NULL) {
		exit(EXIT_FAILURE);
	}

	invoke_monit(opts, configs);

	fprintf(stderr, "monit execution error\n");
	exit(EXIT_FAILURE);
}