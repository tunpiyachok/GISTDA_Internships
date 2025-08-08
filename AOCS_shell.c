//Dispatcher
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // for sleep(), usleep()
#include <pthread.h> // the header file for the pthread lib
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <string.h>//Dispatcher
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // for sleep(), usleep()
#include <pthread.h> // the header file for the pthread lib
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <sys/resource.h>
#include <time.h>
#include "message.h"

Message struct_type = {0};
Message struct_to_send = {0}; // Initialize to 0
Message struct_to_receive = {0};

struct mq_attr attributes = {
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_curmsgs = 0,
    .mq_msgsize = sizeof(Message)
};

void tcc_type(void*arg)
{
	mqd_t mq_GPS = mq_open("/mq_GPS", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //send to TM_Monitor
	if (mq_GPS == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	mqd_t mq_tc_gps = mq_open("/mq_tc_gps", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //send to TC_Monitor
	if (mq_tc_gps == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	/*mqd_t mq_tm_imu = mq_open("/mq_tm_imu", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //send to TM_Monitor
	if (mq_tm_imu == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}*/
	mqd_t mq_imu = mq_open("/mq_imu", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //send to TC_Monitor
	if (mq_imu == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	mqd_t mqdes_type = mq_open("/mq_ttctype", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //receive from sender and Monitor
	if (mqdes_type == -1) 
	{
	    perror("mq_open");
	    exit(EXIT_FAILURE);
	}
	mqd_t mq_return = mq_open("/mq_return_sender", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //return to sender
	if (mq_return == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		if (mq_receive(mqdes_type, (char *)&struct_to_receive, sizeof(Message), NULL) == -1) 
		{
			perror("mq_receive");
			exit(EXIT_FAILURE);
		}
			
		printf("Receive Type : %hhu\n", struct_to_receive.type);
		if (struct_to_receive.type == 0 && struct_to_receive.mdid == 2) 
		{
			struct_to_send = struct_to_receive;
			printf("Send Module ID : %hhu\n", struct_to_send.mdid);
			printf("Send Request ID : %hhu\n", struct_to_send.req_id);
			if (mq_send(mq_GPS, (char *)&struct_to_send, sizeof(struct_to_send), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
			printf("\n-- Wait for respond --\n");
		}
		else if(struct_to_receive.type == 2 && struct_to_receive.mdid == 2)
		{
			struct_to_send = struct_to_receive;
			printf("Send Module ID : %hhu\n", struct_to_send.mdid);
			printf("Send Request ID : %hhu\n", struct_to_send.req_id);
			printf("Send Type : %hhu\n", struct_to_send.type);
			printf("Send Parameter : %hhu\n", struct_to_send.param);
			if (mq_send(mq_tc_gps, (char *)&struct_to_send, sizeof(Message), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
			printf("-------------------------------------------\n");	
		}
		else if (struct_to_receive.type == 0 && struct_to_receive.mdid == 3) 
		{
			struct_to_send = struct_to_receive;
			printf("Send Module ID : %hhu\n", struct_to_send.mdid);
			printf("Send Request ID : %hhu\n", struct_to_send.req_id);
			if (mq_send(mq_imu, (char *)&struct_to_send, sizeof(struct_to_send), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
			printf("\n-- Wait for respond --\n");
		}
		else if(struct_to_receive.type == 2 && struct_to_receive.mdid == 3)
		{
			struct_to_send = struct_to_receive;
			printf("Send Module ID : %hhu\n", struct_to_send.mdid);
			printf("Send Request ID : %hhu\n", struct_to_send.req_id);
			printf("Send Type : %hhu\n", struct_to_send.type);
			printf("Send Parameter : %hhu\n", struct_to_send.param);
			if (mq_send(mq_imu, (char *)&struct_to_send, sizeof(Message), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
			printf("-------------------------------------------\n");	
		}
		else
		{
			printf("Have no request\n");
			Message struct_to_send = {0};
			if (mq_send(mq_return, (char *)&struct_to_send, sizeof(Message), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}
			printf("-------------------------------------------\n");
		}
		Message struct_to_send = {0};
		Message struct_to_receive = {0};
	}
	mq_close(mqdes_type);
	mq_close(mq_GPS);
	mq_close(mq_imu);
	mq_close(mq_tc_gps);
	mq_unlink("/mq_tc_gps");
	mq_unlink("/mq_tc_gps");
	mq_unlink("/mq_GPS");
	mq_unlink("/mq_ttctype");
}

void request_return(void*arg)
{
	mqd_t mq_receive_req = mq_open("/mq_receive_req", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //return to sender
	if (mq_receive_req == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	
	mqd_t mq_return = mq_open("/mq_return_sender", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes); //return to sender
	if (mq_return == -1) 
	{
		perror("mq_open");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		Message struct_to_send = {0};
		Message struct_to_receive = {0};
		if (mq_receive(mq_receive_req, (char *)&struct_to_receive, sizeof(Message), NULL) == -1) 
		{
			perror("mq_receive");
			exit(EXIT_FAILURE);
		}
		struct_to_send = struct_to_receive;
		printf("Receive Type : %hhu\n", struct_to_receive.type);
		printf("Module ID : %u\n", struct_to_send.mdid);
		if(struct_to_receive.type == 1)
		{
		    printf("Telemetry ID : %u\n", struct_to_send.req_id);
			struct_to_send.val;
		}
		else if(struct_to_receive.type == 3)
		{
			printf("Telecommand ID : %hhu\n", struct_to_send.req_id); 
		    struct_to_send.type = 3;
		    printf("Parameter : %hhu\n", struct_to_send.param);
			struct_to_send.val;
		}
		
		else
		{
			printf("Have no Type\n");
			struct_to_send.type;
			printf("Module ID : %u\n", struct_to_send.mdid);
			struct_to_send.req_id = 0;
			printf("-------------------------------------------\n\n");
		}
		
		if (mq_send(mq_return, (char *)&struct_to_send, sizeof(Message), 1) == -1) 
		{
			perror("mq_send");
			exit(EXIT_FAILURE);
		}
		printf("-------------------------------------------\n\n");
		
	}
	mq_close(mq_receive_req);
		mq_close(mq_return);
		mq_unlink("/mq_return_sender");
		mq_unlink("/mq_receive_req");

	
}


int main()
{
	pthread_t type, send;
	
	pthread_create(&type, NULL, (void *(*)(void *))tcc_type, NULL);
	pthread_create(&send, NULL, (void *(*)(void *))request_return, NULL);

	
	pthread_join(type, NULL);
	pthread_join(send, NULL);
}
