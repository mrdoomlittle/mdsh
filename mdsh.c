# include "mdsh.h"
# include <stdio.h>
# include <unistd.h>
# include <malloc.h>
# define FLG_RUNNING 0x1
mdl_u8_t static is_flag(mdl_u8_t __flags, mdl_u8_t __flag) {
	return ((__flags&__flag) == __flag);
}

void mdsh_init(struct mdsh *__mdsh) {
	__mdsh->flags = FLG_RUNNING;
}

void mdsh_de_init(struct mdsh *__mdsh) {
	free(__mdsh->ibuf);
}

mdl_u8_t *rd(mdl_uint_t *__bc, mdl_u8_t *__buf, mdl_uint_t __bs) {
	*__bc = read(fileno(stdin), __buf, __bs);
	*(__buf+(*__bc)-1) = '\0';
	return __buf;
}

# include <string.h>
char *extpart(char *__buf, char *__tmp, mdl_uint_t *__l, mdl_uint_t __bs) {
	while(*__buf == ' ') __buf++;
	char *buf_itr = __buf, *tmp_itr = __tmp;
	while(*buf_itr != ' ' && buf_itr < __buf+__bs && *buf_itr != '\0')
		*(tmp_itr++) = *(buf_itr++);
	*__l = buf_itr-__buf;
	*tmp_itr = '\0';
	char *p = (char*)malloc((*__l)+1);
	memcpy(p, __tmp, (*__l)+1);
	return p;
}

mdl_i8_t is_len(char *__s, mdl_u8_t __l) {
	char *itr = __s;
	for (;*itr != '\0';itr++)
		if (((itr-__s)+1) > __l) return -1;
	return ((itr-__s) == __l);
}

void change_dir(char *__cur_dir, char *__to) {
	if (!is_len(__to, 3)) goto _sk;
	if (((*(mdl_u32_t*)__to)&0xFFFFFF) == (((mdl_u32_t)'/')<<16 | ((mdl_u32_t)'.')<<8 | (mdl_u32_t)'.')) {
		char *itr = __cur_dir, *last = NULL;
		for(;*itr != '\0';itr++)
			if (*itr == '/') last = itr;
		if (last != NULL)
			*last = '\0';
		else {
			//err
		}
		return;
	}

	_sk:
	if (*__to == '/')
		strcpy(__cur_dir, __to);
	else {
		char *end = __cur_dir, *itr;
		for(;*end != '\0';end++);
		*(end++) = '/';
		itr = __to;
		for(;*itr != '\0';itr++)
			*(end++) = *itr;
		*end = '\0';
	}
}

# include <sys/wait.h>
# include <dirent.h>
# include <sys/stat.h>
# include <linux/limits.h>
# include <mdl/str_cmb.h>
# include <fcntl.h>
# include <termios.h>
# include <linux/kd.h>
# include <sys/ioctl.h>
void static bci_exec(char const *__root, char *__file, char *__args) {
	char *ai = __args;
	if (__args != NULL) {
		while(*ai != '\0') {
			if (*ai == ' ') *ai = ',';
			ai++;
		}
	}

	char *argv[] = {"bci", "-exec", __file, NULL, NULL, "-ss", "400", NULL};
	if (__args != NULL) {
		argv[3] = "-args";
		argv[4] = __args;
	}

	pid_t pid;
	char *bci = mdl_str_cmb((char*)__root, mdl_str_cmb("/", "bci/bin/bci", 0), MDL_SC_FREE_RIGHT);
	if ((pid = fork()) == 0) {
		if (execvp(bci, argv) < 0) {
			fprintf(stderr, "failed to exec.\n");
		}
		_exit(0);
	}
	waitpid(pid, NULL, 0);
	free(bci);
}

char static const *help = "\
commands\n\
   cd [dir]\n\
   curd - current path\n\
   ls\n\
   ./ - execute .rbc file\n\
";

char *read_exec_args(char *__ibuf, mdl_uint_t __cc, mdl_uint_t __off) {
	if (__off == __cc) return NULL;
	if (*(__ibuf+__off) == ' ') __off++;
	mdl_uint_t l = (__cc-__off)+1;
	char *s = (char*)malloc(l);
	memcpy(s, __ibuf+__off, l);
	return s;
}

void mdsh_run(struct mdsh *__mdsh, char const *__root, mdl_uint_t __bufsize) {
	__mdsh->ibuf = (char*)malloc(__bufsize);
	char tmp[200];
	memset(__mdsh->ibuf, 0xFF, __bufsize);
	__mdsh->cur_dir = (char*)malloc(PATH_MAX);
	getcwd(__mdsh->cur_dir, PATH_MAX);
	do {
		mdl_uint_t cc, partl, off = 0;
		fprintf(stdout, "%s$ ", __mdsh->cur_dir);
		fflush(stdout);

		rd(&cc, (mdl_u8_t*)__mdsh->ibuf, __bufsize);
		char *base = extpart(__mdsh->ibuf, tmp, &partl, cc);
		if (!partl) goto _end;
		off+= partl;

		if (!strcmp(base, "help")) {
			fprintf(stdout, "%s", help);
		} else if (!strcmp(base, "exit"))
			__mdsh->flags ^= FLG_RUNNING;
		else if (!strcmp(base, "cd")) {
			char *to = extpart(__mdsh->ibuf+off, tmp, &partl, cc);
			off+= partl;
			change_dir(__mdsh->cur_dir, to);
			struct stat st;
			if (stat(__mdsh->cur_dir, &st) == -1)
				fprintf(stderr, "directory does not exits.\n");
			else if (!S_ISDIR(st.st_mode))
				fprintf(stderr, "not a directory.\n");
			free(to);
		} else if (!strcmp(base, "curd")) {
			printf("%s\n", __mdsh->cur_dir);
		} else if (!strcmp(base, "ls")) {
			DIR *d;
			struct dirent *e;
			if ((d = opendir(__mdsh->cur_dir)) != NULL) {
				mdl_u8_t no = 0;
				while((e = readdir(d)) != NULL) {
					if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..") || *e->d_name == '.') continue;
					if ((no>>2)-((no-1)>>2))
						fprintf(stdout, "%s\n", e->d_name);
					else
						fprintf(stdout, "%-30s", e->d_name);
					no++;
				}
				no--;
				if ((no-((no>>2)*(1<<2))) > 0)
					fprintf(stdout, "\n");
			} else
				fprintf(stderr, "failed to list directory.\n");
		} else {
			if (is_len(base, 2) == -1) {
				if (*(mdl_u16_t*)base == (((mdl_u16_t)'/')<<8 | (mdl_u16_t)'.')) {
					char *args = read_exec_args(__mdsh->ibuf, cc, off);
					char *file = mdl_str_cmb(__mdsh->cur_dir, mdl_str_cmb("/", base+2, 0x0), MDL_SC_FREE_RIGHT);
					bci_exec(__root, file, args);
					free(file);
					free(args);
					goto _end;
				}
			}

			char *file = mdl_str_cmb(mdl_str_cmb((char*)__root, "/local/bin", 0x0), mdl_str_cmb("/", base, 0x0), MDL_SC_FREE_BOTH);
			if(!access(file, F_OK)) {
				char *args = read_exec_args(__mdsh->ibuf, cc, off);
				bci_exec(__root, file, args);
				free(args);
			} else {
				fprintf(stdout, "command doesen't exist.\n");
			}
			free(file);
		}
		_end:
		free(base);
	} while(is_flag(__mdsh->flags, FLG_RUNNING));
}
