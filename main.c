# include "mdsh.h"
# include <string.h>
# include <stdio.h>
# include <mdl/qcr.h>
# include <mdlerr.h>
# include <unistd.h>
# include <malloc.h>
char const *conf_file = "config.qc";
int main(int __argc, char const *__argv[]) {
	char const **arg_itr = __argv+1;
	char const *root = NULL;
	while(arg_itr != __argv+__argc) {
		if (!strcmp(*arg_itr, "-root"))
			root = *(++arg_itr);
		arg_itr++;
	}

	mdl_u8_t has_conf = 0;
	if (access(conf_file, F_OK|R_OK) == -1) {
		if (!root) {
			fprintf(stdout, "usage: mdsh -root [path]\n");
			return -1;
		}
	} else {
		struct qcr _qcr;
		if (qcr_init(&_qcr) != MDL_SUCCESS) {
			fprintf(stderr, "failed to init qcr.\n");
			return -1;
		}

		if (qcr_load(&_qcr, (char*)conf_file) != MDL_SUCCESS) {
			fprintf(stderr, "failed to load file.\n");
			return -1;
		}

		if (qcr_read(&_qcr) != MDL_SUCCESS) {
			fprintf(stderr, "failed to read.\n");
			return -1;
		}

		root = (char const*)((struct qcr_val*)qcr_get(&_qcr, "root_dir"))->p;
		if (!root) {
			fprintf(stderr, "somthing went wong.\n");
			return -1;
		}

		if (qcr_de_init(&_qcr) != MDL_SUCCESS) {
			fprintf(stderr, "failed to de init qcr.\n");
			return -1;
		}
		has_conf = 1;
	}

	struct mdsh _msdh;
	mdsh_init(&_msdh);
	mdsh_run(&_msdh, root, 200);
	if (has_conf) free((char*)root);
	return 0;
}
