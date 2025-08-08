#ifndef GPS_H
#define GPS_H

#define MAX_FIELDS 20
#include <time.h>
// ???????????? global ??????????????????
extern char NMEA[MAX_FIELDS][255];
extern int GPS_variable[20];


// ?????????????????????? gps.c ??? TM_GPS.c
void *read_GPS(void *arg);
void *update_variable(void* arg);
void status();
int days_between(struct tm start_date, struct tm end_date);
void GPS_time();
void latitude();
void longitude();
void Altitude();
void Sat_on_Traked();
void HDOP();
void COGs();
void SOG();
int checksum_valid(char *string);
int parse_comma_delimited_str(char *string, char **fields, int max_fields);
int OpenGPSPort(const char *devname);

#endif
