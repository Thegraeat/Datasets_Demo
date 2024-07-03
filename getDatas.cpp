#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <math.h>

#define dt 0.01

struct Data {
    std::string timestamp_sec;
    std::string timestamp_nsec;
    std::string second;             // GPS 周内秒
    std::string week;               // GPS 周
    std::string roll;
    std::string pitch;
    std::string latitude;
    std::string longitude;
    std::string altitude;
    double yaw;
    double enu_velocity_x, enu_velocity_y, enu_velocity_z;     // 东北天速度 [0]e 东 [1]n 北 [2]u 天 (m/s)
    double position_stdev_x, position_stdev_y, position_stdev_z;
    double delta_angle_x, delta_angle_y, delta_angle_z;
    double delta_velocity_x, delta_velocity_y, delta_velocity_z;

};


std::vector<Data> parseData(const std::string &filename) {
    std::ifstream file(filename);
    std::vector<Data> dataEntries;
    std::string line;
    Data data;
    while (std::getline(file, line)) {
        if (line == "---") {
            if (!data.timestamp_sec.empty()) {
                dataEntries.push_back(data);
                data = Data();
            }
        } else if (line.find("sec:") != std::string::npos) {
            data.timestamp_sec = line.substr(line.find(":") + 2);
        } else if (line.find("week:") != std::string::npos){
            data.week = line.substr(line.find(":") + 1);
        } else if (line.find("second:") != std::string::npos) {
            data.second = line.substr(line.find(":") + 2);
        } else if (line.find("latitude:") != std::string::npos) {
            data.latitude = line.substr(line.find(":") + 1);
        } else if (line.find("longitude:") != std::string::npos) {
            data.longitude = line.substr(line.find(":") + 1);
        } else if (line.find("altitude:") != std::string::npos) {
            data.altitude = line.substr(line.find(":") + 1);
        } else if (line.find("position_stdev:") != std::string::npos){
            std::getline(file, line); data.position_stdev_x = std::stod(line.substr(line.find("-") + 1)) * 0.01;
            std::getline(file, line); data.position_stdev_y = std::stod(line.substr(line.find("-") + 1)) * 0.01;
            std::getline(file, line); data.position_stdev_z = std::stod(line.substr(line.find("-") + 1)) * 0.01;
        } else if (line.find("roll:") != std::string::npos) {
            data.roll = line.substr(line.find(":") + 1);
        } else if (line.find("pitch:") != std::string::npos) {
            data.pitch = line.substr(line.find(":") + 1);
        } else if (line.find("yaw:") != std::string::npos) {
            data.yaw = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("enu_velocity:") != std::string::npos){
            std::getline(file, line); data.enu_velocity_x = std::stod(line.substr(line.find(":") + 1));
            std::getline(file, line); data.enu_velocity_y = std::stod(line.substr(line.find(":") + 1));
            std::getline(file, line); data.enu_velocity_z = std::stod(line.substr(line.find(":") + 1));
        } else if (line.find("raw_angular_velocity:") != std::string::npos) {
            std::getline(file, line);data.delta_angle_x = std::stod(line.substr(line.find(":") + 1)) * dt;data.delta_angle_x = data.delta_angle_x * (M_PI / 180);
            std::getline(file, line);data.delta_angle_y = std::stod(line.substr(line.find(":") + 1)) * dt;data.delta_angle_y = data.delta_angle_y * (M_PI / 180);
            std::getline(file, line);data.delta_angle_z = std::stod(line.substr(line.find(":") + 1)) * dt;data.delta_angle_z = data.delta_angle_z * (M_PI / 180);


        } else if (line.find("raw_acceleration:") != std::string::npos) {
            std::getline(file, line); data.delta_velocity_x = std::stod(line.substr(line.find(":") + 1)) * dt;
            std::getline(file, line); data.delta_velocity_y = std::stod(line.substr(line.find(":") + 1)) * dt;
            std::getline(file, line); data.delta_velocity_z = std::stod(line.substr(line.find(":") + 1)) * dt;
        } 
    }
    if (!data.timestamp_sec.empty()) {
        dataEntries.push_back(data);
    }
    return dataEntries;
}

void writeIMUData(const std::vector<Data> &dataEntries, const std::string &filename) {
    std::ofstream file(filename);
    for (const auto &data : dataEntries) {
        file << data.second << " ";
        file << data.delta_angle_y << " " << data.delta_angle_x << " " << -data.delta_angle_z << " ";
        file << data.delta_velocity_y << " " << data.delta_velocity_x << " " << -data.delta_velocity_z << std::endl;
    }
}

void writeGNSSData(const std::vector<Data> &dataEntries, const std::string &filename) {
    std::ofstream file(filename);
    for (const auto &data : dataEntries) {
        file << data.second << " ";
        file << data.latitude << " " << data.longitude << " " << data.altitude << " ";
        file << data.position_stdev_x << " " << data.position_stdev_y << " " << data.position_stdev_z << std::endl;
    }
}

void writeTruthData(const std::vector<Data> &dataEntries, const std::string &filename) {
    std::ofstream file(filename);
    for (const auto &data : dataEntries) {
        file << data.week << " " << data.second << " ";
        file << data.latitude << " " << data.longitude << " " << data.altitude << " ";
        file << data.enu_velocity_y << " " << data.enu_velocity_x << " " << -data.enu_velocity_z << " ";
        file << data.roll << " " << data.pitch << " " << -data.yaw << std::endl;
    }
}

int main() {
    std::vector<Data> dataEntries = parseData("/home/ubuntu/Desktop/datas_demo/datasets07_01_14_21.txt");
    
    writeIMUData(dataEntries, "/home/ubuntu/Desktop/datas_demo/datas/imu_data.txt");
    writeGNSSData(dataEntries, "/home/ubuntu/Desktop/datas_demo/datas/GNSS_data.txt");
    writeTruthData(dataEntries, "/home/ubuntu/Desktop/datas_demo/datas/Truth_data.txt");

    std::cout << "Data extraction complete." << std::endl;
    return 0;
}
