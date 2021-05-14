#include "Errors.h"

namespace Spirmark::Errors
{
	ErrorType lastError = ErrorType::none;
	std::string lastErrorMessage;

	void ReportShaderCodeError(Spirver::ShaderCode& sc)
	{
		lastError = ErrorType::shaderCompilation;
		lastErrorMessage = sc.GetErrors();
	}
}