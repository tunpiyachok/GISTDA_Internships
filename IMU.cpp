#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <pthread.h>
#include <bitset>
#include <cmath>
#include <mqueue.h>
#include <array>
#include <time.h>

#include "IMU.h"
#include "message.h"

struct mq_attr attributes = {
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_msgsize = sizeof(Message),
    .mq_curmsgs = 0
};

int variable[20] = {0};
int TC_value[10] = {0};
int TM_value[10] = {0};

Message struct_to_send = {0};
Message struct_to_receive = {0};

using namespace std;
                                           
std::atomic<int> atom_error(0);
std::atomic<int> atom_status(0);
std::atomic<int> atom_found(0);
std::atomic<uint32_t> atom_accelerometer_X_32bit(0);
std::atomic<uint32_t> atom_accelerometer_Y_32bit(0);
std::atomic<uint32_t> atom_accelerometer_Z_32bit(0);
std::atomic<uint32_t> atom_Gyroscope_X_32bit(0);
std::atomic<uint32_t> atom_Gyroscope_Y_32bit(0);
std::atomic<uint32_t> atom_Gyroscope_Z_32bit(0);
std::atomic<uint32_t> atom_magnetometer_X_32bit(0);
std::atomic<uint32_t> atom_magnetometer_Y_32bit(0);
std::atomic<uint32_t> atom_magnetometer_Z_32bit(0); 
std::atomic<uint32_t> atom_pressure_32bit(0);
std::atomic<uint32_t> atom_temp_32bit(0);
std::atomic<uint32_t> atom_Alt_32bit(0);

class Accelerometer {
private:
    int mpu9250_interface_ = wiringPiI2CSetupInterface(i2c_handle, MPU9250_ADDRESS);
    uint8_t x_high_accelerometer_raw, x_low_accelerometer_raw, y_high_accelerometer_raw, y_low_accelerometer_raw, z_high_accelerometer_raw, z_low_accelerometer_raw;
    int16_t x_accelerometer_Mergedata, y_accelerometer_Mergedata, z_accelerometer_Mergedata;
    float x_accelerometer_con, y_accelerometer_con, z_accelerometer_con;
    pthread_t accel;

    static void* helper(void* arg) {
        Accelerometer* mt = reinterpret_cast<Accelerometer*>(arg);
        mt->Accelerometer_Active();
        return 0;
    }

public:
    void start_accel() {
        pthread_create(&accel, NULL, helper, this);
    }

    void join_accel() {
        pthread_join(accel, NULL);
    }

    void Accelerometer_Active() {
        Accelerometer_Readrawdata();
        Accelerometer_Mergedata();
        Accelerometer_Convertdata();
        Accelerometer_ConvertdataToSignint();
        Accelerometer_Error_Check();
    }

    void Accelerometer_Readrawdata() {
        x_high_accelerometer_raw = wiringPiI2CReadReg8(mpu9250_interface_, ACCEL_XOUT_H);
        x_low_accelerometer_raw = wiringPiI2CReadReg8(mpu9250_interface_, ACCEL_XOUT_L);
        y_high_accelerometer_raw = wiringPiI2CReadReg8(mpu9250_interface_, ACCEL_YOUT_H);
        y_low_accelerometer_raw = wiringPiI2CReadReg8(mpu9250_interface_, ACCEL_YOUT_L);
        z_high_accelerometer_raw = wiringPiI2CReadReg8(mpu9250_interface_, ACCEL_ZOUT_H);
        z_low_accelerometer_raw = wiringPiI2CReadReg8(mpu9250_interface_, ACCEL_ZOUT_L);
    }

    void Accelerometer_Mergedata() {
        x_accelerometer_Mergedata = ((int16_t)(x_high_accelerometer_raw << 8 | x_low_accelerometer_raw));
        y_accelerometer_Mergedata = ((int16_t)(y_high_accelerometer_raw << 8 | y_low_accelerometer_raw));
        z_accelerometer_Mergedata = ((int16_t)(z_high_accelerometer_raw << 8 | z_low_accelerometer_raw));
    }

    void Accelerometer_Convertdata() {
        x_accelerometer_con = ((float)x_accelerometer_Mergedata) / 16384.0;
        y_accelerometer_con = ((float)y_accelerometer_Mergedata) / 16384.0;
        z_accelerometer_con = ((float)z_accelerometer_Mergedata) / 16384.0;
    }

    void Accelerometer_ConvertdataToSignint() {
        atom_accelerometer_X_32bit = static_cast<uint32_t>((x_accelerometer_con * 9.81 + 160) * 100000);
        atom_accelerometer_Y_32bit = static_cast<uint32_t>((y_accelerometer_con * 9.81 + 160) * 100000);
        atom_accelerometer_Z_32bit = static_cast<uint32_t>((z_accelerometer_con * 9.81 + 160) * 100000);
        /*cout << "Accelerometer X: " << atom_accelerometer_X_32bit << endl;
	    cout << "Accelerometer Y: " << atom_accelerometer_Y_32bit << endl;
	    cout << "Accelerometer Z: " << atom_accelerometer_Z_32bit << endl;*/
	    /*cout << "MERGE X Accelerometer: " << bitset<16>(x_accelerometer_Mergedata) << endl;
	    cout << "MERGE Y Accelerometer: " << bitset<16>(y_accelerometer_Mergedata) << endl;
	    cout << "MERGE Z Accelerometer: " << bitset<16>(z_accelerometer_Mergedata) << endl;*/
    }
    
    void Accelerometer_Error_Check()
    {
    	if((atom_accelerometer_X_32bit == atom_accelerometer_Y_32bit) && (atom_accelerometer_Y_32bit == atom_accelerometer_Z_32bit))
    	{
    		atom_error.fetch_add(1);
    		//cout << "error acc: " << atom_error << endl;
		}
	}
};

class Gyroscope {
private:
    int mpu9250_interface_ = wiringPiI2CSetupInterface(i2c_handle, MPU9250_ADDRESS);
    uint8_t x_high_gyro_raw, x_low_gyro_raw, y_high_gyro_raw, y_low_gyro_raw, z_high_gyro_raw, z_low_gyro_raw;
    int16_t x_gyro_Mergedata, y_gyro_Mergedata, z_gyro_Mergedata;
    float x_gyro_con, y_gyro_con, z_gyro_con;
    pthread_t gyro;

    static void* helper(void* arg) {
        Gyroscope* mt = reinterpret_cast<Gyroscope*>(arg);
        mt->Gyroscope_Active();
        return 0;
    }

public:
    void start_gyro() {
        pthread_create(&gyro, NULL, helper, this);
    }

    void join_gyro() {
        pthread_join(gyro, NULL);
    }

    void Gyroscope_Active() {
        Gyroscope_Readrawdata();
        Gyroscope_Mergedata();
        Gyroscope_Convertdata();
        Gyroscope_ConvertdataToSignint();
        Gyroscope_Error_Check();
    }

    void Gyroscope_Readrawdata() {
        x_high_gyro_raw = wiringPiI2CReadReg8(mpu9250_interface_, GYRO_XOUT_H);
        x_low_gyro_raw = wiringPiI2CReadReg8(mpu9250_interface_, GYRO_XOUT_L);
        y_high_gyro_raw = wiringPiI2CReadReg8(mpu9250_interface_, GYRO_YOUT_H);
        y_low_gyro_raw = wiringPiI2CReadReg8(mpu9250_interface_, GYRO_YOUT_L);
        z_high_gyro_raw = wiringPiI2CReadReg8(mpu9250_interface_, GYRO_ZOUT_H);
        z_low_gyro_raw = wiringPiI2CReadReg8(mpu9250_interface_, GYRO_ZOUT_L);
    }

    void Gyroscope_Mergedata() {
        x_gyro_Mergedata = ((int16_t)(x_high_gyro_raw << 8 | x_low_gyro_raw));
        y_gyro_Mergedata = ((int16_t)(y_high_gyro_raw << 8 | y_low_gyro_raw));
        z_gyro_Mergedata = ((int16_t)(z_high_gyro_raw << 8 | z_low_gyro_raw));
    }

    void Gyroscope_Convertdata() {
        x_gyro_con = ((float)x_gyro_Mergedata) / 131.0;
        y_gyro_con = ((float)y_gyro_Mergedata) / 131.0;
        z_gyro_con = ((float)z_gyro_Mergedata) / 131.0;
    }

    void Gyroscope_ConvertdataToSignint() {
        atom_Gyroscope_X_32bit = static_cast<uint32_t>((x_gyro_con + 2000) * 1000);
        atom_Gyroscope_Y_32bit = static_cast<uint32_t>((y_gyro_con + 2000) * 1000);
        atom_Gyroscope_Z_32bit = static_cast<uint32_t>((z_gyro_con + 2000) * 1000);
        /*cout << "Gyroscope X: " << atom_Gyroscope_X_32bit << endl;
	    cout << "Gyroscope Y: " << atom_Gyroscope_Y_32bit << endl;
	    cout << "Gyroscope Z: " << atom_Gyroscope_Z_32bit << endl;*/
	    /*cout << "MERGE X Gyroscope: " << bitset<16>(x_gyro_Mergedata) << endl;
	    cout << "MERGE Y Gyroscope: " << bitset<16>(y_gyro_Mergedata) << endl;
	    cout << "MERGE Z Gyroscope: " << bitset<16>(z_gyro_Mergedata) << endl;*/
    }
    
    void Gyroscope_Error_Check()
    {
    	if((atom_Gyroscope_X_32bit == atom_Gyroscope_Y_32bit) && (atom_Gyroscope_Y_32bit == atom_Gyroscope_Z_32bit))
    	{
    		atom_error.fetch_add(1);
    		//cout << "error gyro: " << atom_error << endl;
		}
	}
};

class Magnetometer {
 private:
  int AK8963_Interface = wiringPiI2CSetupInterface(i2c_handle, AK8963_SLAVE_ADDRESS);

  uint8_t x_high_magnetometer_, x_low_magnetometer_, y_high_magnetometer_, y_low_magnetometer_, z_high_magnetometer_, z_low_magnetometer_;
  int16_t x_raw_magnetometer_, y_raw_magnetometer_, z_raw_magnetometer_;
  int32_t x_32bit_magnetometer_, y_32bit_magnetometer_, z_32bit_magnetometer_;
  float x_magnetometer_, y_magnetometer_, z_magnetometer_;

  /* For magnetometer calibration */
  float mres_, x_calib_, y_calib_, z_calib_, magnetometer_scale_, avg_rad_;
  int magnetometer_max_ = -32767;
  int magnetometer_min_ = 32767;

  pthread_t magnetometer_thread;

  static void* helper(void* arg) {
    Magnetometer* mt = reinterpret_cast<Magnetometer*>(arg);
    mt->Magnetometer_Active();
    return 0;
  }

  void Magnetometer_Active() {
    Magnetometer_Readrawdata();  // Read raw data from sensor
    Magnetometer_Mergedata();    // Merge high bit and low bit from Magnetometer_Readrawdata()
    Magnetometer_Convertdata();  // Convert data from   Magnetometer_Mergedata() to physical value
    Magnetometer_Readrawdata();    // Print physical value
    Magnetometer_ConvertToSignint();  // Convert physical value to int 32 bit
    Magnetometer_Error_Check();
    Magnetometer_Readrawdata_print(); // Print data from Magnetometer_Readrawdata()
    Magnetometer_Mergedata_print(); // Print data from  Magnetometer_Mergedata()
  }

 public:
  	void start_mag() 
	{ 
		pthread_create(&magnetometer_thread, 0, helper, this); 
	}

  	void join_mag() { 
  		pthread_join(magnetometer_thread, 0); 
	}

  void Magnetometer_Readrawdata() {
    x_high_magnetometer_ = wiringPiI2CReadReg8(AK8963_Interface, AK8963_XOUT_H);
    x_low_magnetometer_ = wiringPiI2CReadReg8(AK8963_Interface, AK8963_XOUT_L);
    y_high_magnetometer_ = wiringPiI2CReadReg8(AK8963_Interface, AK8963_YOUT_H);
    y_low_magnetometer_ = wiringPiI2CReadReg8(AK8963_Interface, AK8963_YOUT_L);
    z_high_magnetometer_ = wiringPiI2CReadReg8(AK8963_Interface, AK8963_ZOUT_H);
    z_low_magnetometer_ = wiringPiI2CReadReg8(AK8963_Interface, AK8963_ZOUT_L);
  }

  void Magnetometer_Mergedata() {
    x_raw_magnetometer_ = ((int16_t)(x_high_magnetometer_ << 8 | x_low_magnetometer_));
    y_raw_magnetometer_ = ((int16_t)(y_high_magnetometer_ << 8 | y_low_magnetometer_));
    z_raw_magnetometer_ = ((int16_t)(z_high_magnetometer_ << 8 | z_low_magnetometer_));
  }

  void Magnetometer_Convertdata() {
    mres_ = 4912.0 / 32760.0;
    x_calib_ = ((float)(wiringPiI2CReadReg8(AK8963_Interface, 0x10) - 128) / 256 + 1.0);
    y_calib_ = ((float)(wiringPiI2CReadReg8(AK8963_Interface, 0x11) - 128) / 256 + 1.0);
    z_calib_ = ((float)(wiringPiI2CReadReg8(AK8963_Interface, 0x12) - 128) / 256 + 1.0);

    x_magnetometer_ = (((float)x_raw_magnetometer_ * mres_ * x_calib_));
    y_magnetometer_ = (((float)y_raw_magnetometer_ * mres_ * y_calib_));
    z_magnetometer_ = (((float)z_raw_magnetometer_ * mres_ * z_calib_ ));
  }

  void Magnetometer_ConvertToSignint() {
    atom_magnetometer_X_32bit = static_cast<uint32_t>((x_magnetometer_ + 5000) * 100);
    atom_magnetometer_Y_32bit = static_cast<uint32_t>((y_magnetometer_ + 5000) * 100);
    atom_magnetometer_Z_32bit = static_cast<uint32_t>((z_magnetometer_ + 5000) * 100);
    /*cout << "Magnetometer X: " << atom_magnetometer_X_32bit << endl;
	cout << "Magnetometer Y: " << atom_magnetometer_Y_32bit << endl;
	cout << "Magnetometer Z: " << atom_magnetometer_Z_32bit << endl;*/
  }

  void Magnetometer_Error_Check() 
  {
		if((atom_magnetometer_X_32bit == atom_magnetometer_Y_32bit) && (atom_magnetometer_Y_32bit == atom_magnetometer_Z_32bit))
	    {
	    	atom_error.fetch_add(1);
		}
  }

  void Magnetometer_Readrawdata_print() {
    /*cout << "READ X HIGH Magnetometer: " << bitset<8>(x_high_magnetometer_) << endl;
    cout << "READ X LOW Magnetometer: " << bitset<8>(x_low_magnetometer_) << endl;
    cout << "READ Y HIGH Magnetometer: " << bitset<8>(y_high_magnetometer_) << endl;
    cout << "READ Y LOW Magnetometer: " << bitset<8>(y_low_magnetometer_) << endl;
    cout << "READ Z HIGH Magnetometer: " << bitset<8>(z_high_magnetometer_) << endl;
    cout << "READ Z LOW Magnetometer: " << bitset<8>(z_low_magnetometer_) << endl;*/
  }

  void Magnetometer_Mergedata_print() {
    /*cout << "MERGE X Magnetometer: " << bitset<16>(x_raw_magnetometer_) << endl;
    cout << "MERGE Y Magnetometer: " << bitset<16>(y_raw_magnetometer_) << endl;
    cout << "MERGE Z Magnetometer: " << bitset<16>(z_raw_magnetometer_) << endl;*/
    //cout << "error : " << atom_error << endl;
  }
};

class Pressure {
private:
    int	BMP280_Interface = wiringPiI2CSetupInterface(i2c_handle, BMP280_SLAVE_ADDRESS);
    uint8_t press_lsb, press_msb, press_xlsb, temp_lsb, temp_msb, temp_xlsb;
    int32_t press_raw, temp_raw;
    double adc_T;
    double real_temperature_, real_pressure_;
    float altitude;
    float pressure, temperature;

    pthread_t bmp280;

    static void* helper(void* arg) {
        Pressure* mt = reinterpret_cast<Pressure*>(arg);
        mt->Pressure_Active();
        return 0;
    }

    void Pressure_Active() {
        pressure_read_raw();
        pressure_Mergedata();
        Pressure_Convertdata();

        temp_read_raw();
        temp_Mergedata();
        Temperature_Convertdata();
        
        Altitude_Covertdata();
        
        BMP280_ConvertToSignint();

        //pressure_Readrawdata_print();
        //temp_Readrawdata_print();
    }

public:
    void start_bmp() {
        pthread_create(&bmp280, NULL, helper, this);
    }

    void join_bmp() {
        pthread_join(bmp280, NULL);
    }

    void temp_read_raw() {
        temp_msb = wiringPiI2CReadReg8(BMP280_Interface, 0xFA);
        temp_lsb = wiringPiI2CReadReg8(BMP280_Interface, 0xFB);
        temp_xlsb = wiringPiI2CReadReg8(BMP280_Interface, 0xFC);
    }
    
    void temp_Mergedata() {
        temp_raw = ((int32_t)((temp_msb << 16) | (temp_lsb << 8) | temp_xlsb) >> 4);
    }
    
    void Temperature_Convertdata() {
        double var1, var2;
        var1 = (temp_raw / 16384.0 - 28251 / 1024.0) * 26435;
        var2 = temp_raw / 131072.0 - 28251 / 8192.0;
        adc_T = var1 + var2;
        real_temperature_ = adc_T / 5120.0;
    }
    
    void pressure_read_raw() {
        press_msb = wiringPiI2CReadReg8(BMP280_Interface, 0xF7);
        press_lsb = wiringPiI2CReadReg8(BMP280_Interface, 0xF8);
        press_xlsb = wiringPiI2CReadReg8(BMP280_Interface, 0xF9);
    }

    void pressure_Mergedata() {
        press_raw = ((int32_t)((press_msb << 16) | (press_lsb << 8) | press_xlsb) >> 4);
    }

    void Pressure_Convertdata() {
        double var1, var2, pressure;
        
        // The variable adc_T should have been computed previously in Temperature_Convertdata()
        var1 = (real_temperature_/2.0) - 64000.0;
        var2 = var1 * var1 * -7 / 32768.0;
        var2 = var2 + var1 *140.0 * 2.0;
        var2 = (var2/4.0)+(2855*65536.0);
        var1 = (3024 * var1 * var1 / 524288.0 + (-10685.0) * var1)/524288.0;
        var1 = (1.0+var1/32768.0)*36477;

        
        pressure = 1048576 - press_raw;
        pressure = (pressure-(var2/4096.0))*6250.0/var1;
        var1 = 6000*pressure*pressure/2147483648.0;
        var2 = pressure*(-14600)/32768.0;

        pressure = pressure+(var1+var2+15500)/16.0;
        real_pressure_ = pressure / 256.0;
    }
    
    void Altitude_Covertdata()
    {
    	altitude = (22.2045 / 0.0065) * (1 - pow((real_pressure_/383.9576), 0.190263));
	}
    
    void BMP280_ConvertToSignint() 
	{
		atom_Alt_32bit = static_cast<uint32_t>((altitude + 100) * 100000);
	    atom_pressure_32bit = static_cast<uint32_t>((real_pressure_ + 200) * 100000);
	    atom_temp_32bit = static_cast<uint32_t>((real_temperature_ + 200) * 100);
	    /*cout << "pressure: " << atom_pressure_32bit << endl;
	    cout << "temp: " << atom_temp_32bit << endl;
	    cout << "Altitude: " << atom_Alt_32bit << endl;*/
	}

    /*void pressure_Readrawdata_print() {
        cout << "press_msb: " << bitset<8>(press_msb) << endl;
        cout << "press_lsb: " << bitset<8>(press_lsb) << endl;
        cout << "press_xlsb: " << bitset<8>(press_xlsb) << endl; 
    }*/

    /*void temp_Readrawdata_print() {
          cout << "temp_msb: " << bitset<8>(temp_msb) << endl;
        cout << "temp_lsb: " << bitset<8>(temp_lsb) << endl;
        cout << "temp_xlsb: " << bitset<8>(temp_xlsb) << endl;
    }*/
};

void IMU(Accelerometer& accel, Gyroscope& gyro, Magnetometer& mag, Pressure& pressure) {
  // Start thread
  accel.start_accel();
  gyro.start_gyro();
  mag.start_mag();
  pressure.start_bmp();

  // thread join
  accel.join_accel();
  gyro.join_gyro();
  mag.join_mag();
  pressure.join_bmp();
}

void TC1() {
	int MPU9250_Interface = wiringPiI2CSetup(MPU9250_ADDRESS);
    int AK8963_Interface = wiringPiI2CSetup(AK8963_SLAVE_ADDRESS);    
    int BMP280_Interface = wiringPiI2CSetup(BMP280_SLAVE_ADDRESS);
    
    // Initialize IMU components
    Accelerometer acc;
    Gyroscope gyro;
    Magnetometer mag;
    Pressure pressure;
    
    acc.start_accel();
    gyro.start_gyro();
    mag.start_mag();
    pressure.start_bmp();
    
    acc.join_accel();
    gyro.join_gyro();
    mag.join_mag();
    pressure.join_bmp();
    
    // Config MPU9250
    wiringPiI2CWriteReg8(MPU9250_Interface, INT_PIN_CFG, 0x02);
    wiringPiI2CWriteReg8(AK8963_Interface, AK8963_CNTL1, 0x12); 
    wiringPiI2CWriteReg8(MPU9250_Interface, CONFIG, 0x03);
    wiringPiI2CWriteReg8(MPU9250_Interface, SMPLRT_DIV, 0x04);	
			
	wiringPiI2CWriteReg8(MPU9250_Interface, PWR_MGMT_1, 0x01);  
    wiringPiI2CWriteReg8(MPU9250_Interface, PWR_MGMT_2, 0x00); 	

    // Config BMP280
    wiringPiI2CWriteReg8(BMP280_Interface, BMP_Ctrl, 0x27);
    
    // Prepare filename and check for duplicates
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);

    char filename[100];
    int file_counter = 1;
    FILE *log_file = nullptr;

    do {
        sprintf(filename, "%02d-%02d-%d_%d_IMU_Log.txt", tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900, file_counter);
        log_file = fopen(filename, "r");
        if (log_file != nullptr) {
            fclose(log_file);
            file_counter++;
        }
    } while (log_file != nullptr);

    // Create new file with the updated name
    log_file = fopen(filename, "w");
    if (log_file == nullptr) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    // Write header information
    fprintf(log_file, "Start Log Time: %ld\n", (long)now);
    fprintf(log_file, "1, 0, 1, 3, ""Accelerometer X"", 0, 0.00001, -160, m/s?\n");
    fprintf(log_file, "1, 0, 1, 4, ""Accelerometer Y"", 0, 0.00001, -160, m/s?\n");
    fprintf(log_file, "1, 0, 1, 5, ""Accelerometer Z"", 0, 0.00001, -160, m/s?\n");
    fprintf(log_file, "1, 0, 1, 6, ""Gyroscope X"", 0, 0.001, -2000, d/s\n");
    fprintf(log_file, "1, 0, 1, 7, ""Gyroscope Y"", 0, 0.001, -2000, d/s\n");
    fprintf(log_file, "1, 0, 1, 8, ""Gyroscope Z"", 0, 0.001, -2000, d/s\n");
    fprintf(log_file, "1, 0, 1, 9, ""Magnetometer X"", 0, 0.01, -5000, ?T\n");
    fprintf(log_file, "1, 0, 1, 10, ""Magnetometer Y"", 0, 0.01, -5000, ?T\n");
    fprintf(log_file, "1, 0, 1, 11, ""Magnetometer Z"", 0, 0.01, -5000, ?T\n");
    fprintf(log_file, "1, 0, 1, 12, ""Pressure"", 0, 0.00001, -200, Pa\n");
    fprintf(log_file, "1, 0, 1, 13, ""Temperature"", 0, 0.01, -200, ?C\n");
    fprintf(log_file, "1, 0, 1, 14, ""Altitude"", 0, 0.00001, -100, m\n");

    fflush(log_file);
    
    while (TC_value[0] == 1) {
        // Collect IMU data
        IMU(acc, gyro, mag, pressure);
        
        //cout << "error : " << atom_error << endl;
        
        // Update time
        time(&now);
        tm_info = localtime(&now);
        
        if (atom_error == 0) {
            atom_status = 3;
            atom_found = 1;
        } else if (atom_error == 1 || atom_error.load() == 2) {
            atom_status = 1;
            atom_found = 1;
        } else if (atom_error == 3) {
            atom_status = 0;
            atom_found = 0;
        }
        
        // Write data to file
        fprintf(log_file, "%ld, ", (long)now);
        fprintf(log_file, "%u, ", atom_accelerometer_X_32bit.load());
        fprintf(log_file, "%u, ", atom_accelerometer_Y_32bit.load());
        fprintf(log_file, "%u, ", atom_accelerometer_Z_32bit.load());
        fprintf(log_file, "%u, ", atom_Gyroscope_X_32bit.load());
        fprintf(log_file, "%u, ", atom_Gyroscope_Y_32bit.load());
        fprintf(log_file, "%u, ", atom_Gyroscope_Z_32bit.load());
        fprintf(log_file, "%u, ", atom_magnetometer_X_32bit.load());
        fprintf(log_file, "%u, ", atom_magnetometer_Y_32bit.load());
        fprintf(log_file, "%u, ", atom_magnetometer_Z_32bit.load());
        fprintf(log_file, "%u, ", atom_temp_32bit.load());
        fprintf(log_file, "%u, ", atom_pressure_32bit.load());
        fprintf(log_file, "%u\n", atom_Alt_32bit.load());

        atom_error = 0;
        
        // Flush the file buffer to ensure data is written
        fflush(log_file);
        
        // Sleep for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Close the file after exiting the loop
    fclose(log_file);
}


void TC2() {
    int AK8963_Interface = wiringPiI2CSetup(AK8963_SLAVE_ADDRESS);
    int MPU9250_Interface = wiringPiI2CSetup(MPU9250_ADDRESS);
    int BMP280_Interface = wiringPiI2CSetup(BMP280_SLAVE_ADDRESS);
                                                  
    // Config MPU9250
    wiringPiI2CWriteReg8(AK8963_Interface, AK8963_CNTL1, 0x00);  // Enable Magnetometer sleep mode
    wiringPiI2CWriteReg8(MPU9250_Interface, PWR_MGMT_2, 0x3F);   // Enable Gyroscope and Accelerometer sleep mode

    // Config BMP280
    wiringPiI2CWriteReg8(BMP280_Interface, BMP_Ctrl, 0x3C);      // Enable BMP280 sleep mode
    
    Accelerometer acc;
    Gyroscope gyro;
    Magnetometer mag;
    Pressure pressure;
    
    acc.start_accel();
    gyro.start_gyro();
    mag.start_mag();
    pressure.start_bmp();
    
    acc.join_accel();
    gyro.join_gyro();
    mag.join_mag();
    pressure.join_bmp();
    
    while (TC_value[0] == 0) {
    	IMU(acc, gyro, mag, pressure);
    	
    		cout << "error : " << atom_error << endl;
            
            if (atom_error == 0)
			{
				atom_status = 2;
				atom_found = 1;
			}
			if (atom_error == 1 || atom_error == 2)
			{
				atom_status = 1;
				atom_found = 1;
			}
			else if (atom_error == 3)
			{
				atom_status = 0;
				atom_found = 0;
			}
			
			atom_error = 0;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        /*if (TC_value[0] != 0)
        {
        	wiringPiI2CWriteReg8(MPU9250_Interface, PWR_MGMT_2, 0x00);  // เปิด Gyroscope และ Accelerometer
			wiringPiI2CWriteReg8(MPU9250_Interface, PWR_MGMT_1, 0x01);  // เลือก clock source (Auto select best available clock)
			wiringPiI2CWriteReg8(AK8963_Interface, AK8963_CNTL1, 0x16); 
			
			wiringPiI2CWriteReg8(BMP280_Interface, BMP_Ctrl, 0x27);
			break;
		}*/
    }
}

void Setup()
{
	int MPU9250_Interface = wiringPiI2CSetup(MPU9250_ADDRESS);
	int AK8963_Interface = wiringPiI2CSetup(AK8963_SLAVE_ADDRESS);
    int BMP280_Interface = wiringPiI2CSetup(BMP280_SLAVE_ADDRESS);
                                                  
    // Config MPU9250
    wiringPiI2CWriteReg8(MPU9250_Interface, PWR_MGMT_2, 0x3F);   // Enable Gyroscope and Accelerometer sleep mode
    wiringPiI2CWriteReg8(AK8963_Interface, AK8963_CNTL1, 0x00);  // Enable Magnetometer sleep mode

    // Config BMP280
    wiringPiI2CWriteReg8(BMP280_Interface, BMP_Ctrl, 0x3C);      // Enable BMP280 sleep mode
    
    Accelerometer acc;
    Gyroscope gyro;
    Magnetometer mag;
    Pressure pressure;
    
    acc.start_accel();
    gyro.start_gyro();
    mag.start_mag();
    pressure.start_bmp();
    
    acc.join_accel();
    gyro.join_gyro();
    mag.join_mag();
    pressure.join_bmp();
    
    //cout << "error : " << atom_error << endl;
    
    while(TC_value[0] = 0)
    {
    	IMU(acc, gyro, mag, pressure);
    	if (atom_error == 0)
		{
			atom_status = 2;
			atom_found = 1;
		}
		if (atom_error == 1 || atom_error == 2)
		{
			atom_status = 1;
			atom_found = 1;
		}
		else if (atom_error == 3)
		{
			atom_status = 0;
			atom_found = 0;
		}
		
		atom_error = 0;
	}
    
}

int main()
{
	TC_value[0] = 0;
	
	std::thread setup_IMU;
	setup_IMU = std::thread(Setup);
		
	mqd_t mqdes_send = mq_open("/mq_receive_req", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
	if (mqdes_send == -1) {
	    perror("mq_open");
	    exit(EXIT_FAILURE);
	}
	
	mqd_t mq_imu = mq_open("/mq_imu", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes);
	if (mq_imu == -1) 
	{
	    perror("mq_open");
	    exit(EXIT_FAILURE);
	}
    std::thread tc1_thread;
    std::thread tc2_thread;

    while(true) {
    	Message struct_to_send = {0};
        Message struct_to_receive = {0};
        if (mq_receive(mq_imu, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == -1) {
            perror("mq_receive");
            exit(EXIT_FAILURE);
        }
        
        
		struct_to_send = struct_to_receive;
		
        {	
            if (struct_to_receive.mdid == 3 && struct_to_receive.type == 2 && struct_to_receive.req_id == 1 && TC_value[0] != 1) {
                TC_value[0] = 1;
                struct_to_send.param = TC_value[0];
                
                if (tc1_thread.joinable()) {
                    tc1_thread.join();
                }
                tc1_thread = std::thread(TC1);
                cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
	        	cout << "Parameter : " << static_cast<unsigned int>(struct_to_send.param) << endl;
                std::cout << "TC1 running..." << std::endl;
                struct_to_send.type = 3;
            } 
			else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 2 && struct_to_receive.req_id == 2 && TC_value[0] != 0) {
                TC_value[0] = 0;
                struct_to_send.param = TC_value[0];
                if (tc2_thread.joinable()) {
                    tc2_thread.join();
                }
                tc2_thread = std::thread(TC2);
                cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
	        	cout << "Parameter : " << static_cast<unsigned int>(struct_to_send.param) << endl;
                std::cout << "TC2 running..." << std::endl;
                struct_to_send.type = 3;
            }
            
            /*if (mq_receive(mq_tm_imu, (char *)&struct_to_receive, sizeof(struct_to_receive), NULL) == -1) {
            	perror("mq_receive");
            	exit(EXIT_FAILURE);
        	}*/
        	else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 1) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Find IMU: " << atom_found << endl;
				struct_to_send.val = atom_found;
                struct_to_send.type = 1;
            }
        	else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 2) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Status: " << atom_status << endl;
				struct_to_send.val = atom_status;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 3) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Accelerometer X: " << atom_accelerometer_X_32bit << endl;
				struct_to_send.val = atom_accelerometer_X_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 4) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Accelerometer Y: " << atom_accelerometer_Y_32bit << endl;
                struct_to_send.val = atom_accelerometer_Y_32bit;
                struct_to_send.type = 1;
            }
			else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 5) {
				cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Accelerometer Z: " << atom_accelerometer_Z_32bit << endl;
                struct_to_send.val = atom_accelerometer_Z_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 6) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Gyroscope X: " << atom_Gyroscope_X_32bit << endl;
                struct_to_send.val = atom_Gyroscope_X_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 7) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Gyroscope Y: " << atom_Gyroscope_Y_32bit << endl;
                struct_to_send.val = atom_Gyroscope_Y_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 8) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Gyroscope Z: " << atom_Gyroscope_Z_32bit << endl;
                struct_to_send.val = atom_Gyroscope_Z_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 9) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Magnetometer X: " << atom_magnetometer_X_32bit << endl;
                struct_to_send.val = atom_magnetometer_X_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 10) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Magnetometer Y: " << atom_magnetometer_Y_32bit << endl;
                struct_to_send.val = atom_magnetometer_Y_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 11) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Magnetometer Z: " << atom_magnetometer_Z_32bit << endl;
                struct_to_send.val = atom_magnetometer_Z_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 12) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "pressure: " << atom_pressure_32bit << endl;
                struct_to_send.val = atom_pressure_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 13) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "temp: " << atom_temp_32bit << endl;
                struct_to_send.val = atom_temp_32bit;
                struct_to_send.type = 1;
            }
            else if (struct_to_receive.mdid == 3 && struct_to_receive.type == 0 && struct_to_receive.req_id == 14) {
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
                cout << "Altitude: " << atom_Alt_32bit << endl;
                struct_to_send.val = atom_Alt_32bit;
                struct_to_send.type = 1;
            }
            
            else
            {
            	cout << "Requst has deny" << endl;
            	cout << "Module : " << static_cast<unsigned int>(struct_to_send.mdid) << endl;
	        	cout << "Request : " << static_cast<unsigned int>(struct_to_send.req_id) << endl;
		    	struct_to_send.mdid = 0;
		    	struct_to_send.req_id = 0;
		    	struct_to_send.val = 0;
		    	struct_to_send.type = 3; 
			}
            if (mq_send(mqdes_send, (char *)&struct_to_send, sizeof(struct_to_send), 1) == -1) 
			{
				perror("mq_send");
				exit(EXIT_FAILURE);
			}  
        }

    }
	if (setup_IMU.joinable()) {
	        setup_IMU.join();
	    }
    if (tc1_thread.joinable()) {
        tc1_thread.join();
    }
    if (tc2_thread.joinable()) {
        tc2_thread.join();
    }

    return 0;
}

