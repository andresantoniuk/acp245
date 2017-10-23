/*
 * ACP245 library example code.
 *
 * Tho run this example, you must have the acp245.dll
 * and the license file (license.sig) on the running
 * directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <acp245.h>

 int main(int argc, char* argv[]) {
	u8 buf[256];
    u32 readed;
    u32 written;
    acp_msg msg;
    acp_msg msg_read;
    e_ret rc;
 
	/* 
	 * Initialize the library 
	 * This will search for a license file called
	 * 'license.sig' on the current directory. If you want
	 * to specify an alternative license file, use
	 * acp_init_opts instead.
	 */
	rc = acp_init();
	if (rc != ACP_INIT_OK) {
		printf("Initialization failed: %d. Check your license file.\n", rc);
		getchar();
		return EXIT_FAILURE;
	}

    /* Create a new keep alive message */
    acp_msg_init(&msg, ACP_APP_ID_ALARM, ACP_MSG_TYPE_ALARM_KA);
 
	/* Write the message to a buffer */
    rc = acp_msg_write_data(buf, 256, &written, &msg);
    if (rc != ACP_MSG_OK) {
		printf("Write failed: %d.\n", rc);
    }
 
	/* Read the message from a buffer */
    rc = acp_msg_read_data(buf, written, &readed, &msg_read);
    if(ACP_MSG_OK == rc) {
		printf("ACP Message Application Id is: 0x%X\n", msg_read.hdr.app_id);
        printf("ACP Message Type is: 0x%X\n", msg_read.hdr.type);
	}
	
	acp_msg_free(&msg);
	acp_msg_free(&msg_read);

	/* Just to make sure the console window doesn't close immediately */
    getchar();
	return EXIT_SUCCESS;
}