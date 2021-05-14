#pragma once

#include <filesystem>
#include <BenchmarkedShaderResult.h>
#include <UniformValues.h>
#include <OpenXLSX.hpp>
#include <ShaderToySource.h>
#include <StructuralSimilarity.h>
#include <Helpers/BenchmarkedShader.h>

namespace Spirmark::IO
{

	namespace detail
	{
	
	std::filesystem::path GetCurrentDirectory();

	/// Check if you can save in a directory, with fallback.
	/// Returns: Can the file be saved in the specified location? | In: folder, filename | Out: actualSaveLocation ("" if can't save)
	bool TrySaveFolder(const std::string& folder, const std::string& filename, std::filesystem::path& actualSaveLocation);
	
	void TrimTrailingWhitespace(char* str, int len);
	std::string exec(const char* cmd);
	/// Returns true if at least one variant contains the operation
	bool IsShaderFullStatInteresting(const BenchmarkedShaderResult& sr, int op);
	double RoundDouble(double num, int places); // because the excel decimal places property can't be set from code
	void GetSystemInfo(std::string& os, std::string& cpu);
	/// Returns: number of rows written
	int PrintHeaderToXLSX(const std::vector<std::variant<std::string, int, double>>& headerInfo, OpenXLSX::XLWorksheet& ws);
	void CreateXLSXDoc(const std::filesystem::path& path, const char* worksheetName, OpenXLSX::XLDocument& doc, OpenXLSX::XLWorksheet& ws);
	void GetDateTime(std::string& datetime_pretty, std::string& datetime_filename);

	/// dataOnly: print header (shader name, multi opt iters, recommendation) and simpleStats?
	void WriteShaderResultToXLSX(OpenXLSX::XLWorksheet& ws, const BenchmarkedShaderResult& shaderResult, int& row, bool dataOnly = false);
	void WriteShaderOpCountsToXLSX(OpenXLSX::XLWorksheet& ws, const BenchmarkedShaderResult& shaderResult, int& row);

	extern const std::string perfHeader[4];
	extern const std::string statsSmallHeader[13];

	}

	using namespace detail;

	struct SaveLocations
	{
		std::filesystem::path benchResult, shader, shaderExport, simResult, simImg;

		SaveLocations()
		{
			std::string cdir = GetCurrentDirectory().string();
			benchResult = cdir + "/Results";
			shader = cdir + "/Shaders";
			shaderExport = cdir + "/Exports";
			simResult = cdir + "/Similarity";
			simImg = cdir + "/Similarity";
		}

		SaveLocations(SaveLocations& o) :
			benchResult(o.benchResult), shader(o.shader), shaderExport(o.shaderExport), 
			simResult(o.simResult), simImg(o.simImg) {};
	};
	/// Stores current save locations for the application
	extern SaveLocations saveLocations;

#pragma region 

#pragma region Settings

	void LoadApplicationSettings(int& maxSpirvMultiOptIters, bool& vsync);
	void SaveApplicationSettings(int maxSpirvMultiOptIters, bool vsync);

#pragma endregion

#pragma region Benchmarkable

	/// return false: file already open
	/// byUser: if this is a spirmarkbench file, is it loaded by hand on the ui? if it is, the current benchmark queue will be cleared
	bool LoadBenchmarkable(const std::filesystem::path& fp, bool byUser, std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, float& benchmarkLength);
	void LoadBenchmarkConfig(const std::filesystem::path& fp, bool clearCurrentConfig, std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, float& benchmarkLength);

#pragma endregion

#pragma region Results and Exports

	void ShaderResultsToFile(BenchmarkMode benchmarkMode, const std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, const UniformValues& uniformValues, float benchmarkLength, int spirvMultiOptIterCountMax, int modelCount);
	/// Export all optimized variants
	void ExportShaders(const std::vector<BenchmarkedShader>& benchmarkedShaders, bool uiPopups = true);
	/// Only creates the XLSX, not the images
	void ImageSimilaritiesToFile(const std::vector<std::array<ImageSimilarityStats, shaderProgramTypeCount>>& similarityStats, const std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, float benchmarkLength);

#pragma endregion
}

