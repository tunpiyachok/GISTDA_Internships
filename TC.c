#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include "message.h"
#include "gps.h"
#include <pthread.h>

struct mq_attr attributes = {
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_curmsgs = 0,
    .mq_msgsize = sizeof(Message)
};

Message struct_to_receive = {0};
Message struct_to_send = {0};

int main()
{
    mqd_t mq_telecommand = mq_open("/mq_tc_gps", O_CREAT | O_RDWR, S_IRWXU, &attributes);
    if (mq_telecommand == -1)
    {
        perror("mq_open()");
        exit(1);
    }
    mqd_t mq_Read = mq_open("/mq_telecommand_send", O_CREAT | O_RDWR, S_IRWXU, &attributes);
    if (mq_Read == -1)
    {
        perror("mq_open()");
        exit(1);
    }

    while (1)
    {
        if (mq_receive(mq_telecommand, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == -1)
        {
            perror("mq_receive()");
            exit(1);
        }

        if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 1)
        {
        	struct_to_send = struct_to_receive;
        	struct_to_send.val = 0;
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	        printf("Return : %u\n", struct_to_send.val);
        }
        else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 2)
        {
        	struct_to_send = struct_to_receive;
        	struct_to_send.val = 1;
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	        printf("Return : %u\n", struct_to_send.val);
        }
        else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 3 && (struct_to_receive.param >= 5 && struct_to_receive.param <= 60))
        {
        	struct_to_send = struct_to_receive;
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	        printf("Parameter. : %hhu\n",struct_to_send.param);
        }
        else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 4)
        {
        	struct_to_send = struct_to_receive;
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	        printf("Parameter : %hhu\n",struct_to_send.param);
        }
        else
	    {
	    	printf("Request has deny\n");
	    	printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	struct_to_send.mdid = 0;
	    	struct_to_send.req_id = 0;
	    	struct_to_send.val = 0;
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
		}
        if (mq_send(mq_Read, (char *)&struct_to_send, sizeof(struct_to_send), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}  
        Message struct_to_send = {0};
		Message struct_to_receive = {0};
    }
	mq_close(mq_Read);
	mq_close(mq_telecommand);
    mq_unlink("/mq_telecommand_send");
    mq_unlink("/mq_telecommand");
    	
    return 0;
}

