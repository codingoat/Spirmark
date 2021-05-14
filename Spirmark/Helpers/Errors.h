#pragma once

#include <string>
#include <Spirver/Spirver.h>

namespace Spirmark::Errors
{

enum class ErrorType { none, shaderCompilation, fileAlreadyOpen, benchDone, benchWrongPath, benchCantSave, exportDone, exportWrongPath, exportCantSave, simDone, simWrongPath, simCantSave, simImgDone, simImgWrongPath, simImgCantSave };
extern ErrorType lastError; // stores currently displayed error
extern std::string lastErrorMessage; // stores currently displayed error message
void ReportShaderCodeError(Spirver::ShaderCode& sc);

}