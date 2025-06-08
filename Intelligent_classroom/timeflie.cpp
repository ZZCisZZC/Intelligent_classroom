#include "timefile.h"
#include <fstream>
#include <sstream>

// 创建默认的JSON字符串
std::string createDefaultTimeJson() {
    std::stringstream ss;
    ss << "{\n"
       << "  \"year\": 2025,\n"
       << "  \"month\": 1,\n"
       << "  \"day\": 1,\n"
       << "  \"hour\": 8,\n"
       << "  \"minute\": 30\n"
       << "}";
    return ss.str();
}

std::string getTimeFromFile() {
    const std::string filename = "time.conf";
    std::ifstream file(filename);
    
    // 如果文件不存在，创建一个默认的时间配置
    if (!file.good()) {
        std::string defaultTime = createDefaultTimeJson();
        std::ofstream outFile(filename);
        outFile << defaultTime;
        outFile.close();
        return defaultTime;
    }
    
    // 读取文件内容
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    std::string content = buffer.str();
    if (content.empty()) {
        // 如果文件为空，返回默认时间
        return createDefaultTimeJson();
    }
    
    return content;
}

void storeTimeToFile(std::string time) {
    // 简单验证JSON格式
    if (time.empty() || time[0] != '{' || time[time.length()-1] != '}') {
    }
    
    try {
        std::ofstream file("time.conf");
        file << time;
        file.close();
    } catch (const std::exception& e) {
    }
}

