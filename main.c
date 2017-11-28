# include "mdsh.h"
# include <string.h>
# include <stdio.h>
int main(int __argc, char const *__argv[]) {
	char const **arg_itr = __argv+1;
	char const *root = NULL;
	while(arg_itr != __argv+__argc) {
		if (!strcmp(*arg_itr, "-root"))
			root = *(++arg_itr);
		arg_itr++;
	}

	if (!root) {
		fprintf(stdout, "usage: mdsh -root [path]\n");
		return -1;
	}

	struct mdsh _msdh;
	mdsh_init(&_msdh);
	mdsh_run(&_msdh, root, 200);
	return 0;
}
