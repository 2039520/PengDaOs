#ifndef _FILE_H_PD
#define _FILE_H_PD
#include"lib.h"

#define DIR 0

#define TXT 1

#define CPP 2

#define C   3

#define H   4

#define ASM 5

struct Dir_Items {
	int file_type;
	unsigned char* name;
	struct Dir_Items_Table* parents_DIT;
	struct Dir_Items_Table* child_DIT;
	struct File* file_link;
};
struct Dir_Items_Table {
	struct Dir_Items* parents_DI;
	struct Dir_Items* base_dir_items;
	long length;
	long size;
};
struct FileSystem {
	struct Dir_Items root_DI;
}FileSystem;

void init_dir_items_table(struct Dir_Items_Table* dit, struct Dir_Items* di);

void init_dir_items(struct Dir_Items* di, unsigned char* name, struct Dir_Items* parent_di);

int is_equal(unsigned char* str1, unsigned char* str2);

#endif