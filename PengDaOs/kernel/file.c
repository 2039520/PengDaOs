#include"file.h"
void init_dir_items_table(struct Dir_Items_Table* dit, struct Dir_Items* di) {
	dit->size = 10;
	dit->base_dir_items = (struct Dir_Items*)pd_malloc(sizeof(struct Dir_Items) * dit->size);
	dit->length = 0;
	dit->parents_DI = di;
}

void init_dir_items(struct Dir_Items* di, unsigned char* name, struct Dir_Items* parent_di) {
	di->name = name;
	di->parents_DIT = parent_di->child_DIT;
	struct Dir_Items_Table* dit = (struct Dir_Items_Table*)pd_malloc(sizeof(struct Dir_Items_Table));
	init_dir_items_table(dit, di);
	di->child_DIT = dit;
	di->file_type = DIR;
	di->file_link = NULL;
}

int is_equal(unsigned char* str1, unsigned char* str2) {
	int i = 0;
	while (str1[i] != '\0') {
		if (str1[i] == str2[i]) {
			i++;
			continue;
		}
		else
			return 0;
	}
	if (str2[i] == '\0')
		return 1;
	return 0;
}