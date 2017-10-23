#include "acp245_config.h"
#include <stdio.h>

extern int acp_el_test(void);
extern int acp_msg_test(void);
extern int acp_key_test(void);
extern int acp_license_test(void);

int main (void) {
	acp_el_test();
	acp_msg_test();
	acp_key_test();
	acp_license_test();
	getchar();
}
