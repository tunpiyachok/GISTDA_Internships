#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>
#include <mqueue.h>
#include "gps.h"
#include "message.h"
#include <pthread.h>

void *read_GPS(void *arg);
void *update_variable(void* arg);
int checksum_valid(char *string);
int parse_comma_delimited_str(char *string, char **fields, int max_fields);
int OpenGPSPort(const char *devname);

char NMEA[MAX_FIELDS][255] = {0};
int GPS_variable[20] = {0};

void status()
{   
    if (NMEA[0][0] == 'A') 
    {
        NMEA[0][0] = '1';
    } 
    if (NMEA[0][0] == 'V') 
    {
        NMEA[0][0] = '0';
    }
    if (NMEA[0][0] == '\0') 
    {
        NMEA[0][0] = '\0';
    }
    GPS_variable[0] = NMEA[0][0] - '0';
}


int days_between(struct tm start_date, struct tm end_date) {
    time_t start = mktime(&start_date);
    time_t end = mktime(&end_date);

    double difference = difftime(end, start) / (60 * 60 * 24);
    return (int)difference;
}

void GPS_time()
{
	char UTC_str[10];
	char DATE_str[10];
	
    strcpy(UTC_str, NMEA[1]);
    strcpy(DATE_str, NMEA[2]);

    int UTC = atoi(UTC_str);
    int DATE = atoi(DATE_str);
	
	int hour = (UTC/10000)*3600;
	int min = ((UTC%10000)/100)*60;
	int sec = (UTC%100);
	
	int day = (DATE/10000);
	int month = (DATE%10000)/100;
	int year = (DATE%100)+2000;
	
	struct tm start_date = {0};
    start_date.tm_year = 1970 - 1900; 
    start_date.tm_mon = 0;  
    start_date.tm_mday = 1;
    
	struct tm end_date = {0};
    end_date.tm_year = year - 1900;
    end_date.tm_mon = month-1;  
    end_date.tm_mday = day;
    int days = days_between(start_date, end_date);
    
    year = days*86400;
    GPS_variable[1] = year+hour+min+sec;

}

void latitude()
{
	GPS_variable[8] = NMEA[9][0] - '0';
	
    if (NMEA[3][0] == '\0') {
        return;
    }

    char *token = strtok(NMEA[3], ".");
    if (token == NULL) {
        return;
    }

    char BT[10];
    strcpy(BT, token);

    token = strtok(NULL, ".");
    if (token == NULL) {
        return;
    }

    char AT[10];
    strcpy(AT, token);

    int lat1 = (int) strtol(BT, NULL, 10);
    int lat2 = (int) strtol(AT, NULL, 10);
    float raw_latitude = (int) (lat1 * (int) pow(10, strlen(AT)) + lat2);
    int Flatitude;
    float Blatitude;
    raw_latitude = raw_latitude*0.00001;
    Flatitude = (raw_latitude/100);
    Blatitude = fmod(raw_latitude, 100.0);
    GPS_variable[2] = (Flatitude+(Blatitude/60))*1000000;
    
}

void longitude()
{
	GPS_variable[9] = NMEA[10][0] - '0';

    char *token = strtok(NMEA[4], ".");
    if (token == NULL) {
        return;
    }

    char BG[10];
    strcpy(BG, token);

    token = strtok(NULL, ".");
    if (token == NULL) {
        return;
    }

    char AG[10];
    strcpy(AG, token);

    int lon1 = (int) strtol(BG, NULL, 10);
    int lon2 = (int) strtol(AG, NULL, 10);
    float raw_longitude = (int) (lon1 * (int) pow(10, strlen(AG)) + lon2);
    int Flongitude;
    float Blongitude;
    raw_longitude = raw_longitude*0.00001;
    Flongitude = (raw_longitude/100);
    Blongitude = fmod(raw_longitude, 100.0);
    GPS_variable[3] = (Flongitude+(Blongitude/60))*1000000;
}

void Altitude()
{
    if (NMEA[5][0] == '\0') {
        return;
    }

    char *token = strtok(NMEA[5], ".");
    if (token == NULL) {
        return;
    }

    char BAl[10];
    strcpy(BAl, token);

    token = strtok(NULL, ".");
    if (token == NULL) {
        return;
    }

    char AAl[10];
    strcpy(AAl, token);

    int Alt1 = (int) strtol(BAl, NULL, 10);
    int Alt2 = (int) strtol(AAl, NULL, 10);
    GPS_variable[4] = (int) (Alt1 * (int) pow(10, strlen(AAl)) + Alt2);
}

void Sat_on_Traked()
{
	NMEA[6];

    int Sat = strtol(NMEA[6], NULL, 10);

    GPS_variable[5] = Sat;
}

void HDOP() {
    // Assuming NMEA[8] contains the HDOP value
    char *hdop_str = NMEA[7];

    if (hdop_str == NULL || hdop_str[0] == '\0') {
        // If the string is null or empty, we cannot proceed
        GPS_variable[6] = 0;
        return;
    }

    float hdop = strtof(hdop_str, NULL);
    
    if (hdop == 0 && errno == EINVAL) {
        GPS_variable[6] = 0;
        return;
    }

    GPS_variable[6] = (int)(hdop * 100);
}

void COGs()
{
    char *cogs_str = NMEA[8];

    if (cogs_str == NULL || cogs_str[0] == '\0') {
        GPS_variable[7] = 0;
        return;
    }

    float cogs = strtof(cogs_str, NULL);
    
    if (cogs == 0 && errno == EINVAL) {
        GPS_variable[7] = 0;
        return;
    }

    GPS_variable[7] = (int)(cogs * 100);
}

void SOG() {
    char *sog_str = NMEA[11];
    sleep(1);
    if (sog_str == NULL || sog_str[0] == '\0') {
        GPS_variable[10] = 0;
        return;
    }

    errno = 0; // Clear errno before calling strtof
    float sog = strtof(sog_str, NULL);

    if ((sog == 0 && sog_str == NULL) || errno == ERANGE) {
        GPS_variable[10] = 0;
        return;
    }

    GPS_variable[10] = (int)(sog * 1000);
    GPS_variable[10] = GPS_variable[10]*0.514444;
}

void Log()
{
	// Create new file with the updated name
    log_file = fopen(filename, "w");
    if (log_file == nullptr) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    // Write header information
    fprintf(log_file, "Start Log Time: %ld\n", (long)now);
    fprintf(log_file, "1, 0, 1, 2, ""Time"", 0, 1,0, []\n");
    fprintf(log_file, "1, 0, 1, 3, ""Latitude"", 0, 0.000001, -180, degrees\n");
    fprintf(log_file, "1, 0, 1, 5, ""Longitude"", 0, 0.000001, -90, degrees\n");
    fprintf(log_file, "1, 0, 1, 6, ""Altitude"", 0, 0.01, 0, meter\n");
    fprintf(log_file, "1, 0, 1, 7, ""Satellite on traked"", 0, 1, 0, []\n");
    fprintf(log_file, "1, 0, 1, 8, ""HDOP"", 0, 0.01, 0, []\n");
    fprintf(log_file, "1, 0, 1, 9, ""COGs"", 0, 0.01, 0, degrees\n");
    fprintf(log_file, "1, 0, 1, 10, ""Speed"", 0, 0.001, 0, m/s\n");
    fflush(log_file);
    while(1)
    {
    	time(&now);
        tm_info = localtime(&now);
        
        fprintf(log_file, "%ld, ", (long)now);
		fprintf(log_file, "%u, ", GPS_variable[1]);
		fprintf(log_file, "%u, ", GPS_variable[2]);
		fprintf(log_file, "%u, ", GPS_variable[3]);
		fprintf(log_file, "%hu, ", GPS_variable[4]);
		fprintf(log_file, "%hhu, ", GPS_variable[5]);
		fprintf(log_file, "%hu, ", GPS_variable[6]);
		fprintf(log_file, "%hu, ", GPS_variable[7]);
		fprintf(log_file, "%hu\n", GPS_variable[10]);
		fflush(log_file);
		fclose(log_file);
	}
	/*
	FILE *log_file = fopen("gps_log.txt", "a");
	if (log_file == NULL) {
	    perror("Error opening log file");
	    exit(EXIT_FAILURE);
	}
	fprintf(log_file, "Status : %d\n", GPS_variable[0]);
	fprintf(log_file, "UNIXTIME : %d\n", GPS_variable[1]);
	fprintf(log_file, "Latitude : %f\n", GPS_variable[2]*0.000001);
	fprintf(log_file, "Longitude : %f\n", GPS_variable[3]*0.000001);
	fprintf(log_file, "Altitude : %.2f\n", GPS_variable[4]*0.01);
	fprintf(log_file, "Sat_on_Traked : %d\n", GPS_variable[5]);
	fprintf(log_file, "HDOP : %.2f\n", GPS_variable[6]*0.01);
	fprintf(log_file, "COGs : %.2f\n", GPS_variable[7]*0.01);
	fprintf(log_file, "SOG : %.3f\n\n", GPS_variable[10]*0.001);
	fflush(log_file);
	fclose(log_file);
	*/
}

void *update_variable(void *arg) {
    int fd = OpenGPSPort("/dev/ttyAMA0");
	
	struct mq_attr attributes = {
	    .mq_flags = 0,
	    .mq_maxmsg = 10,
	    .mq_curmsgs = 0,
	    .mq_msgsize = sizeof(Message)
	};
	
    Message struct_to_send = {0};
    Message struct_to_receive = {0};
    mqd_t mq_Read = mq_open("/mq_telecommand_send", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
    if (mq_Read == -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    while (1) {
        status();
        latitude();
	    longitude();
	    printf("%f\n",GPS_variable[2]*0.000001);
	    printf("%f\n",GPS_variable[3]*0.000001);
	    sleep(1);
        /*if (GPS_variable[0] != '\0') {
            GPS_variable[12] = 1;
            

            if (GPS_variable[12] == 1 && GPS_variable[0] == 1) {
                if (mq_receive(mq_Read, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == -1) {
                    perror("mq_receive");
                    exit(EXIT_FAILURE);
                }
            }

            if (struct_to_receive.req_id == 1 && GPS_variable[0] == 1) {
                struct_to_receive.req_id = 0;
                printf("Start Sampling\n");
                while (1) {
                    GPS_variable[0] = 2;
                    GPS_time();
                    latitude();
                    longitude();
                    Altitude();
                    Sat_on_Traked();
                    HDOP();
                    COGs();
                    SOG();
                    struct timespec ts;
                    clock_gettime(CLOCK_REALTIME, &ts);
                    ts.tv_sec += 1; // รอ 1 วินาทีสำหรับ message ใหม่

                    if (mq_timedreceive(mq_Read, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL, &ts) != -1) {
                        if (struct_to_receive.req_id == 2) {
                            printf("Stop Sampling\n");
                            memset(GPS_variable, 0, sizeof(GPS_variable));
                            struct_to_receive.req_id = 0;
                            break;
                        } else if (struct_to_receive.req_id == 3) {
                            struct_to_receive.req_id = 0;
                            printf("Start Log.\n");
                            while (1) {
                                Log();
                                GPS_time();
                                latitude();
                                longitude();
                                Altitude();
                                Sat_on_Traked();
                                HDOP();
                                COGs();
                                SOG();
                                sleep(struct_to_receive.param);
                                clock_gettime(CLOCK_REALTIME, &ts);
                                ts.tv_sec += 0; // ไม่ต้องรอสำหรับ message ใหม่

                                if (mq_timedreceive(mq_Read, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL, &ts) != -1) {
                                    if (struct_to_receive.req_id == 4) {
                                        struct_to_receive.req_id = 0;
                                        struct_to_receive.param = 0;
                                        printf("Stop Log.\n");
                                        break;
                                    }
                                    if (struct_to_receive.req_id == 2) {
                                        printf("Received message with parameter: %hhu\n", struct_to_receive.req_id);
                                        printf("Deactivated\n");
                                        memset(GPS_variable, 0, sizeof(GPS_variable));
                                        struct_to_receive.req_id = 0;
                                        break;
                                    }
                                }
                                printf("Log.\n");
                                sleep(struct_to_receive.param);
                            }
                        }
                    }
                }
            }
        } */
    }

    mq_close(mq_Read);
    mq_unlink("/mq_telecommand_send");

    return NULL;
}


void *read_GPS(void *arg) {
    int fd = -1; // Initialize fd as -1
    char buffer[255];
    int nbytes;
    char *field[MAX_FIELDS];
    int comma;
    if ((fd = OpenGPSPort("/dev/ttyAMA0")) < 0) 
	{
	    printf("Cannot open GPS port\r\n.");
	    return NULL;
	}
	do {
	    if ((nbytes = read(fd, &buffer, sizeof(buffer))) < 0) 
		{
	        perror("Read");
	        return NULL;
	    } 
		else 
		{
	        if (nbytes == 0) 
			{
	            printf("No communication from GPS module\r\n");
	            sleep(1);
	        } 
			else 
			{
	            buffer[nbytes - 1] = '\0';
	            if (checksum_valid(buffer)) 
				{
	                if ((strncmp(buffer, "$GP", 3) == 0)) 
					{
	                	comma = parse_comma_delimited_str(buffer, field, MAX_FIELDS);
	                    	if (strncmp(&buffer[3], "RMC", 3) == 0) 
							{   
		                        strcpy(NMEA[0], field[2]);//Status
		                        strcpy(NMEA[1], field[1]);//UTC
		                        strcpy(NMEA[2], field[9]);//Date
		                        strcpy(NMEA[3], field[3]);//Latitude
		                        strcpy(NMEA[4], field[5]);//Longitude
		                        strcpy(NMEA[8], field[8]);//COGs
		                        strcpy(NMEA[9], field[4]);//Direction N/S
		                        strcpy(NMEA[10], field[6]);//Direction E/W
		                        strcpy(NMEA[11], field[7]);//SOG
		                        printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s*%s\n",field[0],field[1],field[2],field[3],field[4],field[5],field[6],field[7],field[8],field[9],field[10],field[11],field[12],field[13]);
		                    }
		                    if (strncmp(&buffer[3], "GGA", 3) == 0) 
							{
		                        strcpy(NMEA[5], field[9]);//Altitude
		                        strcpy(NMEA[6], field[7]);//SOT
		                        strcpy(NMEA[7], field[8]);//HDOP
		                    }
	                }
	            }
	        }
	    }
	} while(1);
	
	return NULL;
}

int OpenGPSPort(const char *devname)
{
    int fd;
    struct termios options;

    if ((fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
    {
        perror("Open");
        return -1; // Return -1 to indicate failure
    }

    // Set to blocking mode
    fcntl(fd, F_SETFL, 0);

    // Get port attributes
    tcgetattr(fd, &options);

    // Set input and output baud rates to 9600
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // Set 8 bits, no parity, 1 stop bit
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    // Set input mode
    options.c_iflag |= ICRNL;

    // Set local mode
    options.c_lflag &= ~ECHO;
    options.c_lflag |= ICANON;

    // Set port attributes
    tcsetattr(fd, TCSAFLUSH, &options);

    return fd; // Return fd as int
}
int hexchar2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

// Function to convert hexadecimal string to integer
int hex2int(char *c)
{
    int value;
    value = hexchar2int(c[0]);
    value = value << 4;
    value += hexchar2int(c[1]);
    return value;
}

// Function to validate checksum
int checksum_valid(char *string)
{
    char *checksum_str;
    int checksum;
    unsigned char calculated_checksum = 0;

    checksum_str = strchr(string, '*');
    if (checksum_str != NULL) {
        *checksum_str = '\0';
        for (int i = 1; i < strlen(string); i++) {
            calculated_checksum ^= string[i];
        }
        checksum = hex2int(checksum_str + 1);
        if (checksum == calculated_checksum) {
            return 1;
        }
    }
    return 0;
}

// Function to parse comma-delimited string
int parse_comma_delimited_str(char *string, char **fields, int max_fields)
{
    int i = 0;
    fields[i++] = string;
    while ((i < max_fields) && (string = strchr(string, ',')) != NULL) {
        *string = '\0';
        fields[i++] = ++string;
    }
    return i - 1;
}


