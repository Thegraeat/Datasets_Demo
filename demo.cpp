#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits> // 包含此头文件以使用 numeric_limits

// 结构体来存储四元数
struct Quaternion {
    double w, x, y, z;
};

// 结构体来存储位置信息
struct Position {
    double x, y, z;
};


// 将欧拉角转换为四元数
Quaternion eulerToQuaternion(double roll, double pitch, double yaw) {
    double cy = std::cos(yaw * 0.5);
    double sy = std::sin(yaw * 0.5);
    double cp = std::cos(pitch * 0.5);
    double sp = std::sin(pitch * 0.5);
    double cr = std::cos(roll * 0.5);
    double sr = std::sin(roll * 0.5);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

int main() {
    std::ifstream infile("/home/ubentu/Desktop/demo/doc/truth.nav");
    std::ofstream outfile("/home/ubentu/Desktop/demo/doc/tum_format.txt");

    if (!infile.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> lines;

    // 跳过第一行
    std::getline(infile, line);

    while (std::getline(infile, line)) {
        lines.push_back(line);
    }


    for (const auto& line : lines) {
        std::istringstream iss(line);
        double gnss_week, gnss_second, lat, lon, alt, vn, ve, vd, roll, pitch, yaw;
        if (!(iss >> gnss_week >> gnss_second >> lat >> lon >> alt >> vn >> ve >> vd >> roll >> pitch >> yaw)) {
            break;
        }

        double timestamp = gnss_week * 604800 + gnss_second;

        Quaternion quat = eulerToQuaternion(roll, pitch, yaw);

        outfile << std::fixed << std::setprecision(6)
                << timestamp << " "
                << lat<< " " << lon<< " " << alt<< " "
                << quat.x << " " << quat.y << " " << quat.z << " " << quat.w << std::endl;
    }

    infile.close();
    outfile.close();

    // 使用系统命令调用evo_traj进行可视化
    system("evo_traj tum /home/ubentu/Desktop/demo/doc/tum_format.txt --plot");

    return 0;
}
