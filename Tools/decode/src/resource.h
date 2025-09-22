#ifndef RESOURCES_H_
#define RESOURCES_H_
#include <stdint.h>
#include <string>
#include <unordered_map>

extern std::unordered_map<std::string, int> shaders_offset;
extern std::unordered_map<std::string, int> shaders_len;

const uint8_t* ShaderPackage();


#endif









