#ifndef __SAMSUNG_REPORT_COMMON_H__
#define __SAMSUNG_REPORT_COMMON_H__

#define report(fmt, args...)			\
	printf("== REPORT: " fmt" ==\n", ##args);

void report_dfu_env_entities(int argc, char * const argv[]);

#endif /* __SAMSUNG_REPORT_COMMON_H__ */
