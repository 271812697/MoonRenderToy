#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>

// FreeCAD & OpenCASCADE 依赖头
#include "TopoShape.h"
#include "Tools.h"
#include "base/tools.h"
#include <IMeshTools_Parameters.hxx>
#include <BRepTools.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Precision.hxx>
#include <Standard_Version.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>

// 类型别名
using BaseVector3d = Base::Vector3d;
using Facet = Data::ComplexGeoData::Facet;
using Domain = Data::ComplexGeoData::Domain;

// 全局文件夹路径（入参赋值：输入STEP文件夹 / 输出BIN文件夹）
std::string readStepFileDir;
std::string saveStepFileDir;

// 写入单个Vector3d（二进制）
static void WriteVec3(std::ofstream& out, const BaseVector3d& v)
{
    double x = v.x, y = v.y, z = v.z;
    out.write(reinterpret_cast<const char*>(&x), sizeof(double));
    out.write(reinterpret_cast<const char*>(&y), sizeof(double));
    out.write(reinterpret_cast<const char*>(&z), sizeof(double));
}

// 读取单个Vector3d（二进制）
static BaseVector3d ReadVec3(std::ifstream& in)
{
    double x, y, z;
    in.read(reinterpret_cast<char*>(&x), sizeof(double));
    in.read(reinterpret_cast<char*>(&y), sizeof(double));
    in.read(reinterpret_cast<char*>(&z), sizeof(double));
    return BaseVector3d(x, y, z);
}

// 写入三角面索引
static void WriteFacet(std::ofstream& out, const Facet& f)
{
    out.write(reinterpret_cast<const char*>(&f.I1), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&f.I2), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(&f.I3), sizeof(uint32_t));
}

// 读取三角面索引
static Facet ReadFacet(std::ifstream& in)
{
    Facet f{};
    in.read(reinterpret_cast<char*>(&f.I1), sizeof(uint32_t));
    in.read(reinterpret_cast<char*>(&f.I2), sizeof(uint32_t));
    in.read(reinterpret_cast<char*>(&f.I3), sizeof(uint32_t));
    return f;
}

/**
* @brief 批量保存Domain数组至二进制文件
* @param path 输出二进制文件完整路径
* @param domains 网格域数据
* @return 成功返回true
*/
bool SaveDomains(const std::string& path, const std::vector<Domain>& domains)
{
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open())
    {
        std::cerr << "[Error] 无法打开输出文件: " << path << std::endl;
        return false;
    }

    // 写入Domain总数
    uint64_t domainCount = static_cast<uint64_t>(domains.size());
    out.write(reinterpret_cast<char*>(&domainCount), sizeof(uint64_t));

    for (const auto& dom : domains)
    {
        uint64_t ptSize = dom.points.size();
        uint64_t nmlSize = dom.normals.size();
        uint64_t triSize = dom.facets.size();

        // 写入当前Domain顶点数/法向量数/三角面数
        out.write(reinterpret_cast<char*>(&ptSize), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(&nmlSize), sizeof(uint64_t));
        out.write(reinterpret_cast<char*>(&triSize), sizeof(uint64_t));

        // 写入顶点
        for (auto& p : dom.points)  WriteVec3(out, p);
        // 写入法向量
        for (auto& n : dom.normals) WriteVec3(out, n);
        // 写入三角面索引
        for (auto& t : dom.facets)  WriteFacet(out, t);
    }

    out.close();
    std::cout << "[Success] 已转换: " << path << std::endl;
    return true;
}

/**
* @brief 加载单个STEP文件，输出同名BIN文件到指定输出文件夹
* @param stepFilePath 单个STEP文件完整路径
* @param outFolder 输出BIN文件的文件夹路径
*/
void loadTopoShape(const std::string& stepFilePath, const std::string& outFolder)
{
    if (!std::filesystem::exists(stepFilePath) || !std::filesystem::is_regular_file(stepFilePath))
    {
        std::cerr << "[Error] STEP文件无效: " << stepFilePath << std::endl;
        return;
    }

    // 创建输出文件夹（不存在则新建）
    std::filesystem::create_directories(outFolder);

    // 获取原文件名，替换后缀为.bin
    std::string fileName = std::filesystem::path(stepFilePath).stem().string();
    std::string binFilePath = outFolder + "/" + fileName + ".bin";

    // 导入STEP模型
    Part::TopoShape topshape;
    topshape.importStep(stepFilePath.c_str());

    TopoDS_Shape shape = topshape.getShape();
    std::vector<Data::ComplexGeoData::Domain> domains;

    // 提取网格面域
    double accuracy = 0.1 * topshape.getAccuracy();
    topshape.getDomainfaces(domains, accuracy);

    // 保存二进制文件
    SaveDomains(binFilePath, domains);
}

/**
* @brief 遍历文件夹下所有.stp/.step文件，批量转换
* @param inFolder STEP文件输入文件夹
* @param outFolder BIN文件输出文件夹
*/
void BatchConvertStepToBin(const std::string& inFolder, const std::string& outFolder)
{
    if (!std::filesystem::exists(inFolder) || !std::filesystem::is_directory(inFolder))
    {
        std::cerr << "[Error] 输入文件夹不存在或无效: " << inFolder << std::endl;
        return;
    }

    std::cout << "[Info] 开始批量转换，扫描文件夹: " << inFolder << std::endl;

    // 遍历目录所有文件
    for (const auto& entry : std::filesystem::directory_iterator(inFolder))
    {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        // 匹配 .stp / .step 后缀（忽略大小写）
        if (ext == ".stp" || ext == ".step")
        {
            loadTopoShape(entry.path().string(), outFolder);
        }
    }

    std::cout << "[Info] 批量转换任务完成！" << std::endl;
}

int main(int argc, char* argv[])
{
    // 入参校验：输入文件夹、输出文件夹
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << "  [STEP输入文件夹]  [BIN输出文件夹]" << std::endl;
        return 1;
    }

    // 赋值全局文件夹路径
    readStepFileDir = argv[1];
    saveStepFileDir = argv[2];

    // 执行批量转换
    BatchConvertStepToBin(readStepFileDir, saveStepFileDir);

    return 0;
}