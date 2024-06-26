#ifndef _SHELL_PD
#define _SHELL_PD
#include"file.h"
#include"lib.h"
#include"memory.h"
#include"printk.h"
struct Path {
	unsigned char* path;
	int w_length;
	int h_length;
};

struct Shell {
	struct Path work_path;
	struct Dir_Items work_Dir_Items;
}shell;

unsigned char get_elme(struct Path path, int x, int y);

unsigned char* get_row(struct Path path, int x);

void set_elme(struct Path* path, unsigned char elme, int x, int y);

void set_row(struct Path* path, unsigned char* row, int x);

int get_depth(struct Path path);

void init_Path(struct Path* path);

void init_shell();

void print_cursor();

void print_path();

int get_dir_items(struct Path path, struct Dir_Items di_start, struct Dir_Items* di_end);

void make_dir_items(struct Path  path, struct Dir_Items* di_start);

int is_relative_or_absolute(struct Path* path);

void string_to_path(unsigned char* string, struct Path* path);

void init_root_DI();

void do_cmd_cd(struct Path path);

void do_cmd_ls(struct Path path);

void do_cmd_mkdir(struct Path path);

void empty_io_buffer();

void empty_path();

#endif 