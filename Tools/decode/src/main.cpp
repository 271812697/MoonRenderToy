#include "stb_image/stb_image.h"
#include "resource.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
using namespace std;
// 创建文件并写入内容
bool createFile(const std::filesystem::path& filePath, const std::string& content = "") {
    try {
        // 如果目录不存在，则创建目录
        if (!std::filesystem::exists(filePath.parent_path())) {
            std::filesystem::create_directories(filePath.parent_path());
            std::cout << "创建目录: " << filePath.parent_path() << std::endl;
        }

        // 创建并打开文件
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "无法创建文件: " << filePath << std::endl;
            return false;
        }

        // 写入内容（如果有）
        if (!content.empty()) {
            file << content;
        }

        file.close();
        std::cout << "文件创建成功: " << filePath << std::endl;
        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "文件操作错误: " << e.what() << std::endl;
        return false;
    }
}
int main() {

	//int w = -1, h = -1, comp = -1;
	//unsigned char* data = stbi_load("res.png",&w,&h,&comp,4);
	//std::string buf;
	//buf.resize(w*h*comp);
	//memcpy(&buf[0],data,buf.size());
	//ofstream xxdStream(std::string("res.glsl"));
	//xxdStream << buf;

    std::filesystem::path curPath = std::filesystem::current_path();
	const uint8_t* SHADERS_PACKAGE = (ShaderPackage());
	for (auto& it : shaders_offset) {
        std::filesystem::path  tempPath = curPath/it.first;

		int offset = it.second;
		int len = shaders_len[it.first];
		std::string buf;
		buf.resize(len);
		memcpy(&buf[0], &SHADERS_PACKAGE[offset], len);
        createFile(tempPath,buf);
	}

	return 0;
}