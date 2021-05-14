#pragma once


#include <string>
#include <Helpers/ShaderProgramType.h>
#include <ProgramObject.h>
#include <filesystem>
#include <Helpers/BenchmarkMode.h>

namespace Spirmark
{

/// Represents one ShaderToy with all optimization variant statistics, source codes, and OpenGL programs
class BenchmarkedShader {
public:
    BenchmarkedShader(const std::string& name, const std::filesystem::path& path);

    bool Compile(bool model, int spirvMultiOptIterMax);

    std::string GetName() const { return name; }
    bool IsCompiled() const { return compiled; }
    int GetSpirvMultiOptIterMax() const { return spirvMultiOptIterMax; }
    int GetSpirvMultiOptIterUseful() const { return spirvMultiOptIterUseful; } // how many times does running spirvMultiOpt result in a change in the source code?
    const std::array<std::unique_ptr<Spirver::ShaderCode>, shaderProgramTypeCount>& GetShaderCodes() const { return shaderCodes; }
    const std::array<ProgramObject, shaderProgramTypeCount>& GetShaderPrograms() const { return shaderPrograms; }
    const std::array<Spirver::ShaderStat, shaderProgramTypeCount>& GetShaderStats() const { return shaderStats; };

private:
    std::string name;
    int spirvMultiOptIterMax = 0, spirvMultiOptIterUseful = 0;
    bool compiled = false;

    std::array<ProgramObject, shaderProgramTypeCount> shaderPrograms; // programs used by OpenGL
    std::array<std::unique_ptr<Spirver::ShaderCode>, shaderProgramTypeCount> shaderCodes; // source code, to export on demand
    std::array<Spirver::ShaderStat, shaderProgramTypeCount> shaderStats; // statistics (see: SpirverASTAnalyzer)

    // vertex shaders, since BenchmarkedShaders are all fragment shaders
    static void LoadVertexShaders();
    static bool vertexShadersLoaded;
    static ShaderObject *vertFsGlsl, *vertFsSpirv, *vertMGlsl, *vertMSpirv;
};

}