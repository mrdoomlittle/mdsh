# ifndef __mdsh__h
# define __mdsh__h
# include <mdlint.h>
struct mdsh {
	mdl_u8_t flags;
	char *ibuf;
	char *cur_dir;
};

void mdsh_init(struct mdsh*);
void mdsh_de_init(struct mdsh*);
void mdsh_run(struct mdsh*, mdl_uint_t);
# endif
