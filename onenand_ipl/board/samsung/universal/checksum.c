#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

#define IPL_14K	(0x3800 - 0x4)

int main(int argc, char *argv[])
{
	int ret;
	int i, j;
	char buf;
	int fd;
	unsigned int sum = 0;

	fd = open(argv[1], O_RDWR);

	if (fd < 0) {
		printf("open err: %s\n", argv[1]);
		return 1;
	}

	for (i = 0; i < IPL_14K; i++) {
		ret = read(fd, &buf, 1);

		if (ret < 0) {
			printf("write err: %d\n", ret);
			break;
		}

		sum += (buf & 0xFF);

#if 0
		if (i % 16 == 0)
			printf("%04x: ", i);
		printf("%02x ", buf & 0xFF);
		if (i % 16 == 15)
			printf("\n");
#endif
	}

	ret = write(fd, &sum, 4);

	if (ret < 0)
		printf("read err: %s\n", ret);

	close(fd);

	printf("checksum = %x\n", sum);

	return 0;
}
