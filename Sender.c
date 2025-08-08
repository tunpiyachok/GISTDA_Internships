//Sender
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
#include <ctype.h>
#include <time.h>
#include "message.h"

struct mq_attr attributes = {
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_curmsgs = 0,
    .mq_msgsize = sizeof(Message)
};

int is_valid_number(const char *input) {
    while (*input) {
        if (!isdigit((unsigned char)*input)) {
            return 0;
        }
        input++;
    }
    return 1;
}

void* request(void* arg) {
    mqd_t mqdes_send = mq_open("/mq_ttctype", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
    if (mqdes_send == -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    
    while (1) {
        Message struct_to_send = {0};
        char input[100];

        while (1) {
            printf("Enter Type((0) = Telemetry , (2) = Telecommand) : ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';  // Remove newline character
            if (!is_valid_number(input) || sscanf(input, "%hhu", &struct_to_send.type) != 1) {
                printf("Invalid input. Please enter again.\n");
                printf("-------------------------------------------\n\n");
                continue;
            }
            break;
        }

        while (1) {
            printf("Enter Module ID : ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';  // Remove newline character
            if (!is_valid_number(input) || sscanf(input, "%hhu", &struct_to_send.mdid) != 1) {
                printf("Invalid input. Please enter again.\n");
                printf("-------------------------------------------\n\n");
                continue;
            }
            break;
        }

        while (1) {
            printf("Enter Request ID : ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';  // Remove newline character
            if (!is_valid_number(input) || sscanf(input, "%hhu", &struct_to_send.req_id) != 1) {
                printf("Invalid input. Please try again.\n");
                printf("-------------------------------------------\n\n");
                continue;
            }
            break;
        }

        if (struct_to_send.type == 2 && (struct_to_send.req_id != 1 && struct_to_send.req_id != 2 && struct_to_send.req_id != 4)) {
            while (1) {
                printf("Enter Parameter : ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';  // Remove newline character
                if (!is_valid_number(input) || sscanf(input, "%hhu", &struct_to_send.param) != 1) {
                    printf("Invalid input. Please try again.\n");
                    printf("-------------------------------------------\n\n");
                    continue;
                }
                break;
            }
        }

        printf("-------------------------------------------\n\n");
        printf("Send Type : %hhu\n", struct_to_send.type);
        printf("Send Module_ID : %hhu\n", struct_to_send.mdid);
        printf("Send Request_ID : %hhu\n", struct_to_send.req_id);
        if (struct_to_send.type == 2) {
            printf("Send Parameter : %hhu\n", struct_to_send.param);
        }

        if (mq_send(mqdes_send, (char *)&struct_to_send, sizeof(struct_to_send), 1) == -1) {
            perror("mq_send");
            exit(EXIT_FAILURE);
        }
        printf("\n-- Wait for respond --\n");
        sleep(1);
    }

    mq_close(mqdes_send);
    mq_unlink("/mq_ttctype");

    return NULL;
}

void* display(void*arg)
{
	mqd_t mqdes_receive = mq_open("/mq_return_sender", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
	if (mqdes_receive == -1) 
	{
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
		Message struct_to_receive = {0};
		if (mq_receive(mqdes_receive, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == -1) {
            perror("mq_receive");
            exit(EXIT_FAILURE); 
        }
        if (struct_to_receive.type == 3 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 1) 
		{
	        printf("Module : %hhu\n",struct_to_receive.mdid);
	        printf("Request : %hhu\n",struct_to_receive.req_id);
	    	printf("Parameter : %u\n", struct_to_receive.param);
	    }
	    
	    else if (struct_to_receive.type == 3 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 2) 
		{
	        printf("Module : %hhu\n",struct_to_receive.mdid);
	        printf("Request : %hhu\n",struct_to_receive.req_id);
	    	printf("Parameter : %u\n", struct_to_receive.param);
	    }
	    
        else if (struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 1) 
		{
	        printf("Module : %hhu\n",struct_to_receive.mdid);
	        printf("Request : %hhu\n",struct_to_receive.req_id);
	    	printf("Find GPS : %u\n", struct_to_receive.val);
	    }
		if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 2)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("GPS Status : %u\n", struct_to_receive.val);
			printf("-------------------------------------------\n\n");
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 3)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("UNIXTIME : %u\n", struct_to_receive.val);
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 4)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Latitude : %f\n", struct_to_receive.val*0.000001);
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 5)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Longitude : %f\n", struct_to_receive.val*0.000001);
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 6)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Altitude : %.2f\n", struct_to_receive.val*0.01);
			printf("-------------------------------------------\n\n");
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 7)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Satellites Used : %u\n", struct_to_receive.val);
            if(struct_to_receive.val <= 0 || struct_to_receive.val >= 13)
            {
            	printf("Alert : GPS is unusual\n");
			}
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 8)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("HDOP : %.2f\n", struct_to_receive.val*0.01);
            if(struct_to_receive.val <= 0)
            {
            	printf("Alert : GPS is unusual\n");
			}
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 9)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("COGs : %u\n", struct_to_receive.val*0.01);
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 2 && struct_to_receive.req_id == 10)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("SOG : %.3f m/s\n", struct_to_receive.val*0.001);
			printf("-------------------------------------------\n\n");	
		}
		
		//IMU
		//TC_IMU
		else if (struct_to_receive.type == 3 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 1) 
		{
	        printf("Module : %hhu\n",struct_to_receive.mdid);
	        printf("Request : %hhu\n",struct_to_receive.req_id);
	    	printf("Parameter : %u\n", struct_to_receive.param);
	    }
		else if (struct_to_receive.type == 3 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 2) 
		{
	        printf("Module : %hhu\n",struct_to_receive.mdid);
	        printf("Request : %hhu\n",struct_to_receive.req_id);
	    	printf("Parameter : %u\n", struct_to_receive.param);
	    }
	    //TM_IMU
	    else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 1)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Find IMU : %d\n", struct_to_receive.val);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 2)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("IMU Status : %d\n", struct_to_receive.val);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 3)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Accelero X : %.2f m/s²\n", struct_to_receive.val*0.00001-160);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 4)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Accelero Y : %.2f m/s²\n", struct_to_receive.val*0.00001-160);
			printf("-------------------------------------------\n\n");	
		}
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 5)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Accelero Z : %.2f m/s²\n", struct_to_receive.val*0.00001-160);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 6)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Gyro X : %.2f d/s\n", struct_to_receive.val*0.001-2000);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 7)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Gyro Y : %.2f d/s\n", struct_to_receive.val*0.001-2000);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 8)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Gyro Z : %.2f d/s\n", struct_to_receive.val*0.001-2000);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 9)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Magnetic X : %.2f µT\n", struct_to_receive.val*0.01-5000);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 10)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Magnetic Y : %.2f µT\n", struct_to_receive.val*0.01-5000);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 11)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Magnetic Z : %.2f µT\n", struct_to_receive.val*0.01-5000);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 12)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Pressure : %.2f Pa\n", struct_to_receive.val*0.00001-200);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 13)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Temp : %.2f °C\n", struct_to_receive.val*0.01-200);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.type == 1 && struct_to_receive.mdid == 3 && struct_to_receive.req_id == 14)
		{
			printf("Module ID : %hhu\n", struct_to_receive.mdid);
            printf("Telemetry ID : %hhu\n", struct_to_receive.req_id);
            printf("Altitude : %.2f m\n", struct_to_receive.val*0.00001-100);
			printf("-------------------------------------------\n\n");	
		}
		
		else if(struct_to_receive.mdid == 0)
		{
			printf("Request has deny\n");
			printf("-------------------------------------------\n\n");
		}
		
	}
	mq_close(mqdes_receive);
	mq_unlink("/mq_return_sender");
}

int main()
{
	pthread_t sender, receiver;
	
	pthread_create(&sender, NULL, &request, NULL);
	pthread_create(&receiver, NULL, &display, NULL);
	pthread_join(sender, NULL);
	pthread_join(receiver, NULL);
	
	return 0;
}
