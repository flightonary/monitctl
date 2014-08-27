#define _GNU_SOURCE
#include <sys/types.h>
#include <unistd.h>

#define NO_CHANGE -1

int main(int argc, char *argv[])
{
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;
	int ret_getresuid, ret_getresgid;
	int ret_setreuid, ret_setregid;

	// get effective uid and effective gid.
	ret_getresuid = getresuid(&ruid, &euid, &suid);
	ret_getresgid = getresgid(&rgid, &egid, &sgid);

	if(ret_getresuid != 0 || ret_getresgid != 0) {
		fprintf(stderr, "getresuid/getresgid error\n")
	}

	// set effective uid to real uid.
	// set effective gid to real gid.
	ret_setreuid = setreuid(euid, NO_CHANGE);
	ret_setregid = setregid(egid, NO_CHANGE);

	if(ret_setreuid != 0 || ret_setregid != 0) {
		fprintf(stderr, "setreuid/setregid error\n")
	}

	return 0;
}