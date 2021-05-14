#include "BenchmarkedShader.h"
#include <Helpers/Errors.h>
#include <iostream>

namespace Spirmark {

BenchmarkedShader::BenchmarkedShader(const std::string& name, const std::filesystem::path& path) : name(
        name), spirvMultiOptIterMax(spirvMultiOptIterMax) {
    std::string source =
            "#version 460 core\nlayout(location = 0) out vec4 fs_out_col;\nlayout(location = 0) uniform vec3      iResolution;           // viewport resolution (in pixels)\nlayout(location = 1) uniform float     iTime;                 // shader playback time (in seconds)\nlayout(location = 2) uniform float     iTimeDelta;            // render time (in seconds)\nlayout(location = 3) uniform int       iFrame;                // shader playback frame\nlayout(location = 4) uniform float     iChannelTime[4];       // channel playback time (in seconds)\nlayout(location = 8) uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)\nlayout(location = 12) uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click\n"
            + Spirver::proc::fileToString(path.string().c_str())
            + "\nvoid main(){ mainImage(fs_out_col, gl_FragCoord.xy); }";

    shaderCodes[ShaderProgramType::glsl] = std::make_unique<Spirver::GlslShader>(
            Spirver::GlslShader::FromMemory(source, Spirver::Stage::Fragment));
}

bool BenchmarkedShader::Compile(bool model, int spirvMultiOptIterMax) {
    if(!vertexShadersLoaded) LoadVertexShaders();

    this->spirvMultiOptIterMax = spirvMultiOptIterMax;
    ShaderObject *shaderObjects[ShaderProgramType::shaderProgramTypeCount];

    auto& glslShader = dynamic_cast<Spirver::GlslShader &>(*shaderCodes[0]);

    Spirver::SpirvShader spirvShader = glslShader.ToSpirv(); // glslShader is not optimized yet
    if (spirvShader.HasErrors()) {
        Errors::ReportShaderCodeError(spirvShader);
        return false;
    }
    shaderObjects[spirv] = new ShaderObject(spirvShader);
    shaderStats[spirv] = spirvShader.Analyze();
    shaderCodes[spirv] = std::make_unique<Spirver::SpirvShader>(Spirver::SpirvShader(spirvShader));

    shaderObjects[glsl] = new ShaderObject(glslShader);
    shaderStats[glsl] = glslShader.Analyze();

    // glslOpt
    if (!glslShader.Optimize()) {
        Errors::ReportShaderCodeError(glslShader);
        return false;
    }
    shaderObjects[glslOpt] = new ShaderObject(glslShader);
    shaderStats[glslOpt] = glslShader.Analyze();
    shaderCodes[glslOpt] = std::make_unique<Spirver::GlslShader>(Spirver::GlslShader(glslShader));

    // spirvOpt
    if (!spirvShader.Optimize()) {
        Errors::ReportShaderCodeError(spirvShader);
        return false;
    }
    shaderObjects[spirvOpt] = new ShaderObject(spirvShader);
    shaderStats[spirvOpt] = spirvShader.Analyze();
    shaderCodes[spirvOpt] = std::make_unique<Spirver::SpirvShader>(Spirver::SpirvShader(spirvShader));

    // spirvMultiOpt
    Spirver::SpirvShader spirvMultiOptShader = spirvShader.ToGlsl().ToSpirv();
    Spirver::ShaderStat lastSpirvOptStat, newSpirvOptStat = shaderStats[spirvOpt];
    spirvMultiOptIterUseful = 0;
    while (++spirvMultiOptIterUseful < spirvMultiOptIterMax && newSpirvOptStat != lastSpirvOptStat) {
        lastSpirvOptStat = newSpirvOptStat;
        spirvMultiOptShader.Optimize();
        spirvMultiOptShader = spirvMultiOptShader.ToGlsl().ToSpirv();
        newSpirvOptStat = spirvMultiOptShader.Analyze();
    }
    if(newSpirvOptStat == lastSpirvOptStat) spirvMultiOptIterUseful--; // was the last iteration useless?

    shaderObjects[spirvMultiOpt] = new ShaderObject(spirvMultiOptShader);
    shaderStats[spirvMultiOpt] = newSpirvOptStat;
    shaderCodes[spirvMultiOpt] = std::make_unique<Spirver::SpirvShader>(Spirver::SpirvShader(spirvMultiOptShader));

    // glslOptSpirvOpt
    Spirver::SpirvShader doubleOptShader = glslShader.ToSpirv();
    if (!doubleOptShader.Optimize()) {
        Errors::ReportShaderCodeError(doubleOptShader);
        return false;
    }
    shaderObjects[glslOptSpirvOpt] = new ShaderObject(doubleOptShader);
    shaderStats[glslOptSpirvOpt] = doubleOptShader.Analyze();
    shaderCodes[glslOptSpirvOpt] = std::make_unique<Spirver::SpirvShader>(Spirver::SpirvShader(doubleOptShader));

    // spirvOptGlsl
    Spirver::GlslShader spirvOptGlslShader = spirvShader.ToGlsl(); // the last time this was modified was during spirvOpt
    if (spirvOptGlslShader.HasErrors()) {
        Errors::ReportShaderCodeError(spirvOptGlslShader);
        return false;
    }
    shaderObjects[spirvOptGlsl] = new ShaderObject(spirvOptGlslShader);
    shaderStats[spirvOptGlsl] = spirvOptGlslShader.Analyze();
    shaderCodes[spirvOptGlsl] = std::make_unique<Spirver::GlslShader>(Spirver::GlslShader(spirvOptGlslShader));


    // attach and link
    for (int progType = 0; progType < shaderProgramTypeCount; progType++) {
        shaderPrograms[progType] = ProgramObject();
        if (!model) // fullscreen
        {
            if (progType >= 2 && progType <= 5) shaderPrograms[progType].AttachShader(*vertFsSpirv);
            else shaderPrograms[progType].AttachShader(*vertFsGlsl);
        } else {
            if (progType >= 2 && progType <= 5) shaderPrograms[progType].AttachShader(*vertMSpirv);
            else shaderPrograms[progType].AttachShader(*vertMGlsl);
        }
        shaderPrograms[progType].AttachShader(*shaderObjects[progType]);
        if (!shaderPrograms[progType].LinkProgram()) std::cout << "TODO: link failed" << std::endl;
        delete shaderObjects[progType];
    }

    compiled = true;
    return true;
}

    bool BenchmarkedShader::vertexShadersLoaded = false;
    ShaderObject *BenchmarkedShader::vertFsGlsl = nullptr,
        *BenchmarkedShader::vertFsSpirv = nullptr,
        *BenchmarkedShader::vertMGlsl = nullptr,
        *BenchmarkedShader::vertMSpirv = nullptr;


    void BenchmarkedShader::LoadVertexShaders() {
        vertFsGlsl = new ShaderObject(Spirver::StageToGlsl(Spirver::Stage::Vertex), "Assets/fullscreen.vert");
        vertFsSpirv = new ShaderObject(ShaderObject::LoadGlslAsSpirv("Assets/fullscreen.vert", Spirver::Stage::Vertex));
        vertMGlsl = new ShaderObject(Spirver::StageToGlsl(Spirver::Stage::Vertex), "Assets/model.vert");
        vertMSpirv = new ShaderObject(ShaderObject::LoadGlslAsSpirv("Assets/model.vert", Spirver::Stage::Vertex));
        vertexShadersLoaded = true;
    }

} // Spirmark

