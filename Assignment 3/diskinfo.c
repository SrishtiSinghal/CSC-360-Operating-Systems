#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	
    int fd = open(argv[1], O_RDWR);
		
    struct stat buffer;
    int status = fstat(fd, &buffer);

    char* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	int block_size = 0, block_count = 0, fat_starts = 0;
	int fat_blocks = 0, root_start = 0, root_blocks = 0;

	int free_block_count = 0, res_block_count = 0, allo_block_count = 0, cur_block_val = 0;
	
	memcpy(&block_size, address+8, 2);
	block_size = htons(block_size);
	memcpy(&block_count, address+10, 4);
	block_count = htonl(block_count);
	memcpy(&fat_starts, address+14, 4);
	fat_starts = htonl(fat_starts);
	memcpy(&fat_blocks, address+18, 4);
	fat_blocks = htonl(fat_blocks);
	memcpy(&root_start, address+22, 4);
	root_start = htonl(root_start);
	memcpy(&root_blocks, address+26, 4);
	root_blocks = htonl(root_blocks);

	
	int cur_block = fat_starts * block_size;
	int i = 0; 
	while(i<block_count){
		memcpy(&cur_block_val, address + cur_block, 4); 
		cur_block_val = htonl(cur_block_val);
		switch (cur_block_val) {
			case 0: 
				free_block_count++;
				break;
			case 1: 
				res_block_count++;
				break;

			default: 
				allo_block_count++;
				break;
		}
		i++;
		cur_block = cur_block + 4;
	}

    munmap(address,buffer.st_size);

    close(fd);
	
	printf("Super block information:\n"
			"Block size: %d\n"
			"Block count: %d\n"
			"FAT starts: %d\n"
			"FAT blocks: %d\n"
			"Root directory start: %d\n"
			"Root directory blocks: %d\n\n"
			"FAT information:\n"
			"Free Blocks: %d\n"
			"Reserved Blocks: %d\n"
			"Allocated Blocks: %d\n",
			block_size, block_count, fat_starts, fat_blocks, root_start, root_blocks, free_block_count,
			res_block_count, allo_block_count);	
	
	block_size++;
}