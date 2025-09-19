#include "stb_image/stb_image.h"
#include "resource.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
int main() {
	//int w = -1, h = -1, comp = -1;
	//unsigned char* data = stbi_load("res.png",&w,&h,&comp,4);
	//std::string buf;
	//buf.resize(w*h*comp);
	//memcpy(&buf[0],data,buf.size());
	//ofstream xxdStream(std::string("res.glsl"));
	//xxdStream << buf;

	const char* SHADERS_PACKAGE = reinterpret_cast<const char*>(ShaderPackage());
	for (auto& it : shaders_offset) {
		int offset = it.second;
		int len = shaders_len[it.first];
		std::string buf;
		buf.resize(len);
		memcpy(&buf[0],&SHADERS_PACKAGE[offset],len);
		std::ostringstream oss;
		ofstream xxdStream(it.first);
		xxdStream << buf;
	}

	return 0;
}