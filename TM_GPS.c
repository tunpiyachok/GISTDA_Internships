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
Message struct_to_send = {0};
Message struct_to_receive = {0};

int main()
{	
	pthread_t GPS,update;
	
	pthread_attr_t attr;
	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	pthread_create(&update, &attr, &update_variable, NULL);
	pthread_create(&GPS, NULL, &read_GPS, NULL);
	
	mqd_t mq_GPS = mq_open("/mq_GPS", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
	if (mq_GPS == -1) 
	{
	    perror("mq_open");
	    exit(EXIT_FAILURE);
	}
	mqd_t mqdes_send = mq_open("/mq_receive_req", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
	if (mqdes_send == -1) {
	    perror("mq_open");
	    exit(EXIT_FAILURE);
	}
	while (1)
	{
		Message struct_to_send = {0};
		Message struct_to_receive = {0};
		if (mq_receive(mq_GPS, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == -1) 
		{
			perror("mq_receive");
			exit(EXIT_FAILURE);
		}
		struct_to_send = struct_to_receive;
		if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 1) 
		{
	        struct_to_send.val = GPS_variable[12];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("Find GPS : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1;
	    	printf("-------------------------------------------\n\n");
	    }
		else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 2) 
		{
	        struct_to_send.val = GPS_variable[0];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("Status : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1;
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 3) 
		{
	        struct_to_send.val = GPS_variable[1];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("UNIXTIME : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 4) 
		{
			struct_to_send.val = GPS_variable[2];
			struct_to_send.param = GPS_variable[8];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("Latitude : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 5) 
		{
			struct_to_send.val = GPS_variable[3];
			struct_to_send.param = GPS_variable[9];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("Longitude : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 6) 
		{
			struct_to_send.val = GPS_variable[4];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("Altitude : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 7) 
		{
			struct_to_send.val = GPS_variable[5];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("Satellites Used : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 8) 
		{
			struct_to_send.val = GPS_variable[6];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("HDOP : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 9) 
		{
			struct_to_send.val = GPS_variable[7];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("COGs : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if (struct_to_receive.mdid == 2 && struct_to_receive.req_id == 10) 
		{
			struct_to_send.val = GPS_variable[10];
	        printf("Module : %hhu\n",struct_to_send.mdid);
	        printf("Request : %hhu\n",struct_to_send.req_id);
	    	printf("SOG : %u\n", struct_to_send.val);
	    	struct_to_send.type = 1; 
	    	printf("-------------------------------------------\n\n");
	    }
	    else if(struct_to_receive.mdid != 2)
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
	    if (mq_send(mqdes_send, (char *)&struct_to_send, sizeof(struct_to_send), 1) == -1) 
		{
			perror("mq_send");
			exit(EXIT_FAILURE);
		}  
	}
	mq_close(mq_GPS);
	mq_close(mqdes_send);
	mq_unlink("/mq_GPS");
	mq_unlink("/mq_receive_req"); 
	
	pthread_join(GPS, NULL);
	return 0;
}
