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

// 将地理坐标转换为ECEF坐标
Position geodeticToECEF(double lat, double lon, double h) {
    double a = 6378137.0; // 地球长半轴
    double e2 = 6.69437999014e-3; // 第一偏心率平方
    lat = lat * M_PI / 180.0;
    lon = lon * M_PI / 180.0;
    double N = a / std::sqrt(1 - e2 * std::sin(lat) * std::sin(lat));
    Position pos;
    pos.x = (N + h) * std::cos(lat) * std::cos(lon);
    pos.y = (N + h) * std::cos(lat) * std::sin(lon);
    pos.z = (N * (1 - e2) + h) * std::sin(lat);
    return pos;
}

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
    std::ifstream infile("/home/ubentu/Desktop/demo/doc/OB_GINS_TXT.nav");
    std::ofstream outfile("/home/ubentu/Desktop/demo/doc/OB_GINS_TXT.txt");

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

    // 用第一行的数据作为参考原点
    std::istringstream ref_iss(lines[0]);
    double ref_lat, ref_lon, ref_alt;
    ref_iss.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // 跳过GNSS周
    ref_iss.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // 跳过GNSS每周秒数
    ref_iss >> ref_lat >> ref_lon >> ref_alt;

    Position ref_pos = geodeticToECEF(ref_lat, ref_lon, ref_alt);

    double scale_factor = 1; // 缩放因子
    double bigger_factor = 1000;

    for (const auto& line : lines) {
        std::istringstream iss(line);
        double gnss_week, gnss_second, lat, lon, alt, vn, ve, vd, roll, pitch, yaw;
        if (!(iss >> gnss_week >> gnss_second >> lat >> lon >> alt >> vn >> ve >> vd >> roll >> pitch >> yaw)) {
            break;
        }

        double timestamp = gnss_week * 604800 + gnss_second;
        Position pos = geodeticToECEF(lat, lon, alt);

        // 转换到局部坐标系并进行缩放
        pos.x = (pos.x - ref_pos.x) * scale_factor;
        pos.y = (pos.y - ref_pos.y) * scale_factor;
        pos.z = (pos.z - ref_pos.z) * scale_factor;

        Quaternion quat = eulerToQuaternion(roll, pitch, yaw);

        outfile << std::fixed << std::setprecision(6)
                << timestamp << " "
                << (pos.x-(-7736796.621433))*0.1<< " " << (pos.y-(1798696.594430))*0.1<< " " << (pos.z-(3985638.722107))*0.1<< " "
                << quat.x << " " << quat.y << " " << quat.z << " " << quat.w << std::endl;
    }

    infile.close();
    outfile.close();

    // 使用系统命令调用evo_traj进行可视化
    system("evo_traj tum /home/ubentu/Desktop/demo/doc/OB_GINS_TXT.txt --plot");

    return 0;
}
