#include "e_libs_config.h"
#include <stdio.h>

extern int e_conf_test(void);
extern int e_util_test(void);
extern int e_timer_test(void);
extern int e_syslog_test(void);
extern int e_queue_test(void);
extern int e_buff_test(void);
extern int e_crc_test(void);
extern int e_log_test(void);

int main (void) {
	e_conf_test();
	e_util_test();
	e_timer_test();
	e_syslog_test();
	e_queue_test();
	e_buff_test();	
	e_crc_test();
	e_log_test();
	getchar();
}
