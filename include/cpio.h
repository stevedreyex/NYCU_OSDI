#ifndef _CPIO_H
#define _CPIO_H

void initrd_list();
void cat_list ();
void callback_initramfs(void * addr);
int get_initramfs();
unsigned int * load_prog (void * prog);
int get_initramfs_size();
void print_initramfs();

#endif
