# include "mdsh.h"
int main(void) {
	struct mdsh _msdh;
	mdsh_init(&_msdh);
	mdsh_run(&_msdh, 200);
}
