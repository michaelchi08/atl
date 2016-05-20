#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#include <navio2/Ublox.h>
#include <navio2/Util.h>

#include "imu.hpp"


// CONSTANTS
#define IMU_RECORD_FILE "imu.dat"
#define GPS_RECORD_FILE "gps.dat"


// FUNCTIONS
static void writeIMURecordFileHeader(std::ofstream &imu_file)
{
    imu_file << "t,us,";

    imu_file << "mpu9250_ax,mpu9250_ay,mpu9250_az,";
    imu_file << "mpu9250_gx,mpu9250_gy,mpu9250_gz,";
    imu_file << "mpu9250_mx,mpu9250_my,mpu9250_mz,";
    imu_file << "lsm9ds1_ax,lsm9ds1_ay,lsm9ds1_az,";
    imu_file << "lsm9ds1_gx,lsm9ds1_gy,lsm9ds1_gz,";
    imu_file << "lsm9ds1_mx,lsm9ds1_my,lsm9ds1_mz" << std::endl;
}

static void writeGPSRecordFileHeader(std::ofstream &gps_file)
{
    gps_file << "t,us,";

    gps_file << "gps_ms_of_week,";
    gps_file << "gps_long,";
    gps_file << "gps_lat,";
    gps_file << "gps_height_ellipsoid,";
    gps_file << "gps_height_mean_sea_level,";
    gps_file << "gps_horizontal_accuracy_estimate,";
    gps_file << "gps_vertical_accuracy_estimate" << std::endl;
}

static void recordIMUData(IMU &imu, std::ofstream &imu_file, struct timeval &now)
{
    float data[3];

    // record time according to raspberry pi in seconds and micro seconds
    imu_file << now.tv_sec << ",";
    imu_file << now.tv_usec << ",";

    // MPU9250 accelerometer (x, y, z)
    imu.mpu9250->read_accelerometer(&data[0], &data[1], &data[2]);
    imu_file << data[0] << ",";
    imu_file << data[1] << ",";
    imu_file << data[2] << ",";

    // MPU9250 gyroscope (x, y, z)
    imu.mpu9250->read_gyroscope(&data[0], &data[1], &data[2]);
    imu_file << data[0] << ",";
    imu_file << data[1] << ",";
    imu_file << data[2] << ",";

    // MPU9250 magnetometer (x, y, z)
    imu.mpu9250->read_magnetometer(&data[0], &data[1], &data[2]);
    imu_file << data[0] << ",";
    imu_file << data[1] << ",";
    imu_file << data[2] << ",";

    // LSM9DS1 accelerometer (x, y, z)
    imu.lsm9ds1->read_accelerometer(&data[0], &data[1], &data[2]);
    imu_file << data[0] << ",";
    imu_file << data[1] << ",";
    imu_file << data[2] << ",";

    // LSM9DS1 gyroscope (x, y, z)
    imu.lsm9ds1->read_gyroscope(&data[0], &data[1], &data[2]);
    imu_file << data[0] << ",";
    imu_file << data[1] << ",";
    imu_file << data[2] << ",";

    // LSM9DS1 magnetometer (x, y, z)
    imu.lsm9ds1->read_magnetometer(&data[0], &data[1], &data[2]);
    imu_file << data[0] << ",";
    imu_file << data[1] << ",";
    imu_file << data[2] << std::endl;
}

void recordGPSData(std::vector<double> pos_data, std::ofstream &gps_file, struct timeval &now)
{
    // record time according to raspberry pi in seconds and micro seconds
    if ((int) pos_data[0] == 0x03) {
        gps_file << now.tv_sec << ",";
        gps_file << now.tv_usec << ",";

        gps_file << pos_data[1] / 10000000 << ",";
        gps_file << pos_data[2] / 10000000 << ",";
        gps_file << pos_data[3] / 1000 << ",";
        gps_file << pos_data[4] / 1000 << ",";
        gps_file << pos_data[5] / 1000 << ",";
        gps_file << pos_data[6] / 1000 << std::endl;

    } else {
        std::cout << "Not in 3D-fix mode" << std::endl;
    }
}

static void *logIMUData(void *args)
{
    IMU imu;
    std::ofstream imu_file;
    struct timeval now;

    // setup
    imu.initialize();
    imu_file.open(IMU_RECORD_FILE);
    writeIMURecordFileHeader(imu_file);

    // record imu data
    while (1) {
        imu.update();
        // imu.print();

        gettimeofday(&now, NULL);
        recordIMUData(imu, imu_file, now);
    }

    // clean up
    imu_file.close();

    return NULL;
}

static void *logGPSData(void *args)
{
    Ublox gps;
    int msg_ok;
    struct timeval now;
    std::ofstream gps_file;
    std::vector<double> pos_data;

    // setup
    gps_file.open(GPS_RECORD_FILE);
    writeGPSRecordFileHeader(gps_file);

    // record gps data
    while (1) {
        gettimeofday(&now, NULL);
        msg_ok = gps.decodeSingleMessage(Ublox::NAV_POSLLH, pos_data);

        if (msg_ok == 1 && pos_data[0] == 0x03) {
            recordGPSData(pos_data, gps_file, now);
        } else if (msg_ok == 1 && pos_data[0] != 0x03) {
            std::cout << "Lost 3D Fix" << std::endl;
        }
    }

    // clean up
    gps_file.close();

    return NULL;
}

int main(int argc, char **argv)
{
    Ublox gps;
    std::vector<double> pos_data;

    // check gps
    if (gps.testConnection()) {
        std::cout << "Connected to GPS!" << std::endl;
    } else {
        std::cout << "Error! GPS not connected!" << std::endl;
        exit(1);
    }

    // don't start logging until in 3D-fix mode
    std::cout << "Waiting for 3D fix! " << std::endl;
    while (1) {
        if (gps.decodeSingleMessage(Ublox::NAV_POSLLH, pos_data) == 1) {
            if ((int) pos_data[0] != 0x03) {
                std::cout << ".";
            } else {
                std::cout << "\nGot 3D fix! continuing!" << std::endl;
                break;
            }
        }
        sleep(1);
    }

    // log data
    pthread_t imu_thread;
    pthread_t gps_thread;

    pthread_create(&imu_thread, NULL, logIMUData, NULL);
    pthread_create(&gps_thread, NULL, logGPSData, NULL);

    pthread_join(imu_thread, NULL);
    pthread_join(gps_thread, NULL);

    return 0;
}
