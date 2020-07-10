#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_BIT(var,pos) ((var) & (1<<(pos))) 

int main(int argc, char* argv[]) {
	
    int fd = open(argv[1], O_RDWR);
		
    struct stat buffer;
    int status = fstat(fd, &buffer);

    char* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	int block_size = 0, block_count = 0, fat_starts = 0;
	int fat_blocks = 0, root_start = 0, root_blocks = 0;
	
	char file_name[31] = "";
	int year = 0, mon = 0, day = 0;
	int hour = 0, min = 0, second = 0;

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

	int pos_start = root_start * block_size;
	int pos_start_val = 0;
	int i = 0;
	while(i<root_blocks*8){
		memcpy(&pos_start_val, address + pos_start, 1);
		if(CHECK_BIT(pos_start_val, 0)){
			if(CHECK_BIT(pos_start_val, 1)){
				printf("F ");
			}
			else if(CHECK_BIT(pos_start_val, 2)){
				printf("D ");
			}
			memcpy(&pos_start_val, address + pos_start + 9, 4);
			pos_start_val = htonl(pos_start_val);
			memcpy(&file_name, address + pos_start + 27, 31);
			printf("%10d", pos_start_val);
			printf("%30s ", file_name);
			memcpy(&mon, address + pos_start + 22, 1);
			memcpy(&day, address + pos_start + 23, 1);
			memcpy(&hour, address + pos_start + 24, 1);
			memcpy(&min, address + pos_start + 25, 1);
			memcpy(&second, address + pos_start + 26, 1);
			memcpy(&year, address + pos_start + 20, 2);
			year = htons(year);

			printf("%02d/%02d/%02d %02d:%02d:%02d\n", year, mon, day, hour, min, second);	

		}
		i++;
		pos_start = pos_start + 64;
	} 

	munmap(address, buffer.st_size);
	close(fd);
}
