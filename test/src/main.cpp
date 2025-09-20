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

	const uint8_t* SHADERS_PACKAGE = (ShaderPackage());
	for (auto& it : shaders_offset) {
		ofstream xxdStream(it.first);
		int offset = it.second;
		int len = shaders_len[it.first];
		std::string buf;
		buf.resize(len);
		memcpy(&buf[0], &SHADERS_PACKAGE[offset], len);
		xxdStream << buf;
		//std::ostringstream oss;
		//int cnt = 0;
		//for (int i = 0; i < len;) {


		//	//if (SHADERS_PACKAGE[offset + i] == 10)
		//	if (0)
		//	{
		//		cnt++;
		//		if (cnt % 2) {
		//			oss << SHADERS_PACKAGE[offset + i];
		//		}
		//		i++;
		//		//while (i < len && (SHADERS_PACKAGE[offset + i] == 10 || SHADERS_PACKAGE[offset + i] == 13))
		//		//{
		//			//i++;
		//		//}
		//	}
		//	else
		//	{
		//		oss << SHADERS_PACKAGE[offset + i];
		//		i++;
		//	}


		//}

		//std::string a = oss.str();
		//xxdStream << oss.str();
	}

	return 0;
}