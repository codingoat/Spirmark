#include "IO.h"

#ifdef _WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#include <intrin.h>
#else
#include <unistd.h>
#include <cpuid.h>
#define GetCurrentDir getcwd
#endif
#include <fstream>
#include <Errors.h>


namespace Spirmark::IO
{

	namespace detail
	{

	std::filesystem::path GetCurrentDirectory()
	{
		static std::filesystem::path path;
		if (path.string() != "") return path;

		char cdir[512] = { 0 };
		GetCurrentDir(cdir, 512);
		path = std::filesystem::path(cdir);
		return path;
	}

	bool TrySaveFolder(const std::string& folder, const std::string& filename, std::filesystem::path& actualSaveLocation)
	{
		actualSaveLocation = folder + "/" + filename;
		std::ofstream file(actualSaveLocation);
		bool canSaveFile = true;
		if (!file.is_open())
		{
			canSaveFile = false;
			actualSaveLocation = GetCurrentDirectory().string() + "/" + filename;
			file = std::ofstream(actualSaveLocation);

			if (!file.is_open())
			{
				actualSaveLocation = std::filesystem::path("");
				return false;
			}
		}
		file.close();
		std::remove(actualSaveLocation.string().c_str());
		actualSaveLocation.make_preferred();
		return canSaveFile;
	}

	void TrimTrailingWhitespace(char* str, int len)
	{
		int i = len - 1;
		auto isContent = [str](int i) {return str[i] != 0 && str[i] != ' '; };
		while (i > 0 && !isContent(i)) i--;
		if (i >= 0 && i < len - 1) str[i + 1] = 0;
	}

	std::string exec(const char* cmd) {
	#ifndef _WIN32
			std::array<char, 128> buffer;
			std::string result;
			std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
			if (!pipe) {
				throw std::runtime_error("popen() failed!");
			}
			while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
				result += buffer.data();
			}
			return result;
	#else
			return "";
	#endif
	}

	bool IsShaderFullStatInteresting(const BenchmarkedShaderResult& sr, int op)
	{
		for (int i = 0; i < shaderProgramTypeCount; i++)
			if (sr.stat[i].opCounts[op]) return true;
		return false;
	}

	double RoundDouble(double num, int places)
	{
		int a = std::pow(10, places);
		return (int)(num * a) / (double)a;
	}

	void GetSystemInfo(std::string& os, std::string& cpu)
	{
	#ifdef  _WIN32
		int cpuInfo[4] = { -1 };
		char CPUBrandString[64] = { 0 };
		//memset(CPUBrandString, 0, sizeof(CPUBrandString));
		__cpuid(cpuInfo, 0x80000002);
		memcpy(CPUBrandString, cpuInfo, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x80000003);
		memcpy(CPUBrandString + 16, cpuInfo, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x80000004);
		memcpy(CPUBrandString + 32, cpuInfo, sizeof(cpuInfo));
		TrimTrailingWhitespace(CPUBrandString, 64);
		cpu = std::string(CPUBrandString);

		os = "Windows";
	#else
		cpu = exec("cat /proc/cpuinfo | grep \"model name\" | head -1 | cut -d':' -f2- | cut -b 2-");

		os = "Linux";
	#endif
	}

	int PrintHeaderToXLSX(const std::vector<std::variant<std::string, int, double>>& headerInfo, OpenXLSX::XLWorksheet& ws)
	{
		int row = 1, col = 1; // imagine indexing from 1

		for (auto h : headerInfo)
		{
			if (std::holds_alternative<std::string>(h))
			{
				std::string val = std::get<std::string>(h);
				if (val[0] == '\n')
				{
					row++;
					col = 1;
				}
				else ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = val;

			}
			else if (std::holds_alternative<int>(h))
				ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = std::get<int>(h);
			else
				ws.cell(OpenXLSX::XLCellReference(row, col++)).value() = std::get<double>(h);
		}

		return row;
	}

	void CreateXLSXDoc(const std::filesystem::path& path, const char* worksheetName, OpenXLSX::XLDocument& doc, OpenXLSX::XLWorksheet& ws)
	{
		using namespace OpenXLSX;
		doc.create(path.string());
		doc.workbook().addWorksheet(worksheetName);
		doc.workbook().deleteSheet("Sheet1");
		ws = doc.workbook().worksheet(worksheetName);
	}

	void GetDateTime(std::string& datetime_pretty, std::string& datetime_filename)
	{
		std::stringstream datetime_fn, datetime_nice;
		std::time_t t = std::time(nullptr);
		std::tm tm = *std::localtime(&t);
		datetime_fn << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
		datetime_nice << std::put_time(&tm, "%Y. %m. %d. %H:%M:%S");
		datetime_pretty = datetime_nice.str();
		datetime_filename = datetime_fn.str();
	}

	void WriteShaderResultToXLSX(OpenXLSX::XLWorksheet& ws, const BenchmarkedShaderResult& shaderResult, int& row, bool dataOnly)
	{
		using namespace OpenXLSX;
		using CellRef = XLCellReference;

		int col = 1;
		
		if (!dataOnly)
		{
			ws.cell(CellRef(row++, col)).value() = shaderResult.name;
			ws.cell(CellRef(row, col++)).value() = "Useful SpirvMultiOpt iterations";
			ws.cell(CellRef(row++, col--)).value() = shaderResult.usefulSpirvMultiOptIters;
			ws.cell(CellRef(row, col++)).value() = "Recommended compilation method";
			ws.cell(CellRef(row++, col--)).value() = shaderProgramTypeNames[shaderResult.GetRecommendedProgramType()];
			row++;
		}

		col = 2;
		for (auto& str : perfHeader) ws.cell(CellRef(row, col++)).value() = str;
		row++;

		for (int p = glsl; p < shaderProgramTypeCount; p++) // benchmark
		{
			ws.cell(CellRef(row, 1)).value() = shaderProgramTypeNames[p];
			ws.cell(CellRef(row, 2)).value() = RoundDouble(shaderResult.results[p].mean.total, 2);
			ws.cell(CellRef(row, 3)).value() = RoundDouble(shaderResult.results[p].stdev.total, 3);
			ws.cell(CellRef(row, 4)).value() = RoundDouble(shaderResult.results[p].mean.gpu, 2);
			ws.cell(CellRef(row, 5)).value() = RoundDouble(shaderResult.results[p].stdev.gpu, 3);
			row++;
		}
		row++;

		// simple stats
		if(!dataOnly)
		{
			col = 2;
			for (auto str : Spirver::shaderStatNames) ws.cell(CellRef(row, col++)).value() = str;
			row++;

			for (int p = glsl; p < shaderProgramTypeCount; p++) // shader stats
			{
				ws.cell(CellRef(row, 1)).value() = shaderProgramTypeNames[p];
				for (int j = 0; j < Spirver::shaderStatTypesCount; j++)
					ws.cell(CellRef(row, j + 2)).value() = shaderResult.stat[p].stats[j];
				row++;
			}

			row += 2;
		}
	}

	void WriteShaderOpCountsToXLSX(OpenXLSX::XLWorksheet& ws, const BenchmarkedShaderResult& shaderResult, int& row)
	{
		using namespace OpenXLSX;
		using CellRef = XLCellReference;

		int col = 1;
		ws.cell(CellRef(row++, col++)).value() = shaderResult.name + " opcounts";
		for (int i = 0; i < 791; i++)
			if (IsShaderFullStatInteresting(shaderResult, i)) // skip full 0 columns
				ws.cell(CellRef(row, col++)).value() = Spirver::glslangOperatorNames[i];;
		row++;
		for (int p = glsl; p < shaderProgramTypeCount; p++)
		{
			col = 1;
			ws.cell(CellRef(row, col++)).value() = shaderProgramTypeNames[p];
			for (int i = 0; i < 791; i++)
				if (IsShaderFullStatInteresting(shaderResult, i)) // skip full 0 columns
					ws.cell(CellRef(row, col++)).value() = shaderResult.stat[p].opCounts[i];
			row++;
		}
		row++;
	}


		const std::string perfHeader[4] = { "AVG","STDEV","GPU AVG","GPU STDEV" };
		const std::string statsSmallHeader[13] = { "glslLines""spirvLines","math","mathExpensive","conditional","assign","access","compare","functionDef","functionCall","tempVar","loop","flow","switch" };
	} // detail


using namespace Spirmark::IO::detail;

SaveLocations saveLocations;

#pragma region Settings

void LoadApplicationSettings(int& maxSpirvMultiOptIters, bool& vsync)
{
	std::ifstream file("./spirmark.ini");
	if (!file.is_open()) return;
	std::string line;
	auto getSettingPath = [&file, &line](std::filesystem::path& setting) {if (!std::getline(file, line)) return false; setting = line; return true; };

	if (!std::getline(file, line)) return;
	maxSpirvMultiOptIters = std::stoi(line);
	if (!std::getline(file, line)) return;
	vsync = (bool)std::stoi(line);
	if (!getSettingPath(saveLocations.benchResult)) return;
	if (!getSettingPath(saveLocations.shader)) return;
	if (!getSettingPath(saveLocations.shaderExport)) return;
	if (!getSettingPath(saveLocations.simResult)) return;
	if (!getSettingPath(saveLocations.simImg)) return;

	file.close();
}

void SaveApplicationSettings(int maxSpirvMultiOptIters, bool vsync)
{
	std::ofstream file("./spirmark.ini");
	if (!file.is_open()) return;

	file << maxSpirvMultiOptIters << std::endl << (vsync ? 1 : 0) << std::endl
		<< saveLocations.benchResult.string() << std::endl
		<< saveLocations.shader.string() << std::endl
		<< saveLocations.shaderExport.string() << std::endl
		<< saveLocations.simResult.string() << std::endl
		<< saveLocations.simImg.string();

	file.close();
}

#pragma endregion

#pragma region Benchmarkable

bool LoadBenchmarkable(const std::filesystem::path& fp, bool byUser, std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, float& benchmarkLength)
{
	// check path
	std::ifstream file(fp);
	if (!file.is_open()) return false;

	std::string name = fp.filename().string(), path = fp.string(), extension = fp.extension().string();

	if (extension == ".spirmark") LoadBenchmarkConfig(fp, byUser, benchmarkedShaderResults, benchmarkLength);
	else
	{
		auto samename = std::find_if(benchmarkedShaderResults.begin(), benchmarkedShaderResults.end(), [&name](const BenchmarkedShaderResult& x) { return x.name == name; });
		if (samename == benchmarkedShaderResults.end()) benchmarkedShaderResults.emplace_back(name, path);
		else
		{
			Errors::lastError = Errors::ErrorType::fileAlreadyOpen;
			Errors::lastErrorMessage = name;
			return false;
		}
	}

	return true;
}

void LoadBenchmarkConfig(const std::filesystem::path& fp, bool clearCurrentConfig, std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, float& benchmarkLength)
{
	std::ifstream file(fp);
	if (!file.is_open()) return;

	std::string line;
	std::getline(file, line);
	int length = line == "" ? 0 : std::stoi(line);
	if (length > 0) benchmarkLength = length;

	if (clearCurrentConfig) benchmarkedShaderResults.clear();
	while (std::getline(file, line))
		LoadBenchmarkable(line, false, benchmarkedShaderResults, benchmarkLength); // byUser: false, otherwise the already loaded shaders from this config would be removed

	file.close();
}

#pragma endregion

#pragma region Result export

void ShaderResultsToFile(BenchmarkMode benchmarkMode, const std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, const UniformValues& uniformValues, float benchmarkLength, int spirvMultiOptIterCountMax, int modelCount)
{
	using namespace OpenXLSX;
	using CellRef = XLCellReference;

	std::string datetime_fn, datetime_nice;
	GetDateTime(datetime_nice, datetime_fn);

	std::filesystem::path actualSaveLocation;
	bool originalFolder = TrySaveFolder(saveLocations.benchResult.string(), "spirmark_" + datetime_fn + ".xlsx", actualSaveLocation);
	if (!originalFolder && actualSaveLocation.string() == "")
	{
		Errors::lastError = Errors::ErrorType::benchCantSave;
		return;
	}

	XLDocument doc; XLWorksheet ws;
	CreateXLSXDoc(actualSaveLocation, "BenchmarkResults", doc, ws);

	// header
	std::string cpuModel, os;
	GetSystemInfo(os, cpuModel);
	std::vector<std::variant<std::string, int, double>> headerInfo = {
		"GPU Vendor", (const char*)glGetString(GL_VENDOR), "\n",
		"GPU Model", (const char*)glGetString(GL_RENDERER), "\n",
		"CPU", cpuModel, "\n",
		"OS", os, "\n",
		"Resolution", uniformValues.resolution.x, uniformValues.resolution.y, "\n",
		"Datetime", datetime_nice, "\n",
		"Max spirvMultiOpt iterations", spirvMultiOptIterCountMax, "\n",
		"Model count (if in model mode)", modelCount, "\n",
		"Total benchmark time (m)", RoundDouble(benchmarkLength, 1), "\n",
		"Individual benchmark time (m)", benchmarkMode != MultiModel ? RoundDouble(benchmarkLength / benchmarkedShaderResults.size() / shaderProgramTypeCount, 1) : RoundDouble(benchmarkLength / shaderProgramTypeCount, 1), "\n",
		"Benchmark mode", benchmarkModeNames[benchmarkMode], "\n", "\n"
	};
	int row = PrintHeaderToXLSX(headerInfo, ws), col = 1;

	// performance (& simple stats)
	if (benchmarkMode != BenchmarkMode::MultiModel)
	{
		for (const BenchmarkedShaderResult& sr : benchmarkedShaderResults)
			WriteShaderResultToXLSX(ws, sr, row);
	}
	else // multimodel: only one common perf statistic
	{
		ws.cell(CellRef(row++, col)).value() = "Benchmarked shaders:";
		row++;
		for (const BenchmarkedShaderResult& sr : benchmarkedShaderResults)
			ws.cell(CellRef(row++, col)).value() = sr.name;
		row++;
		WriteShaderResultToXLSX(ws, benchmarkedShaderResults[0], row, true);
	}

	// opCounts
	for (const BenchmarkedShaderResult& sr : benchmarkedShaderResults)
		WriteShaderOpCountsToXLSX(ws, sr, row);

	doc.save();
	doc.close();

	Errors::lastError = originalFolder ? Errors::ErrorType::benchDone : Errors::ErrorType::benchWrongPath;
	Errors::lastErrorMessage = actualSaveLocation.string();
}

void ExportShaders(const std::vector<BenchmarkedShader>& benchmarkedShaders, bool uiPopups)
{
	std::filesystem::path actualSaveLocation;
	bool originalFolder = TrySaveFolder(saveLocations.benchResult.string(), ".temp", actualSaveLocation);
	if (!originalFolder && actualSaveLocation.string() == "")
	{
		Errors::lastError = Errors::ErrorType::exportCantSave;
		return;
	}
	actualSaveLocation = actualSaveLocation.parent_path().string() + "/";

	for (int shader = 0; shader < benchmarkedShaders.size(); shader++)
	{
		std::string shaderName(benchmarkedShaders[shader].GetName().begin(), benchmarkedShaders[shader].GetName().end() - 5); // cut extension
		// using name from loadedShaderToys is fine, since you can only export when that and shaderCodes are in sync

		for (int program = 0; program < shaderProgramTypeCount; program++)
		{
			std::string ext = program < 2 || program == 6 ? ".frag" : ".spv";
			benchmarkedShaders[shader].GetShaderCodes()[program]->ToFile(actualSaveLocation.string() + shaderName + "_" + shaderProgramTypeNames[program] + ext);
		}
	}

	if (uiPopups)
	{
		Errors::lastError = originalFolder ? Errors::ErrorType::exportDone : Errors::ErrorType::exportWrongPath;
		Errors::lastErrorMessage = actualSaveLocation.string();
	}
}

void ImageSimilaritiesToFile(const std::vector<std::array<ImageSimilarityStats, shaderProgramTypeCount>>& similarityStats, const std::vector<BenchmarkedShaderResult>& benchmarkedShaderResults, float benchmarkLength)
{
	using namespace OpenXLSX;
	using CellRef = XLCellReference;

	std::string datetime_fn, datetime_nice;
	GetDateTime(datetime_nice, datetime_fn);

	std::filesystem::path actualSaveLocation;
	bool originalFolder = TrySaveFolder(saveLocations.simResult.string(), "similarity_" + datetime_fn + ".xlsx", actualSaveLocation);
	if (!originalFolder && actualSaveLocation.string() == "")
	{
		Errors::lastError = Errors::ErrorType::simCantSave;
		return;
	}

	XLDocument doc; XLWorksheet ws;
	CreateXLSXDoc(actualSaveLocation, "Similarity", doc, ws);

	// header
	std::string cpuModel, os;
	GetSystemInfo(os, cpuModel);
	std::vector<std::variant<std::string, int, double>> headerInfo = {
		"GPU Vendor", (const char*)glGetString(GL_VENDOR), "\n",
		"GPU Model", (const char*)glGetString(GL_RENDERER), "\n",
		"CPU", cpuModel, "\n",
		"OS", os, "\n",
		"Resolution", similarityStats[0][0].worstUniformValues.resolution.x, similarityStats[0][0].worstUniformValues.resolution.y, "\n",
		"Datetime", datetime_nice, "\n",
		"Total benchmark time (m)", RoundDouble(benchmarkLength, 1), "\n",
		"Individual benchmark time (m)", RoundDouble(benchmarkLength / benchmarkedShaderResults.size() / shaderProgramTypeCount, 1), "\n", "\n"
	};
	int row = PrintHeaderToXLSX(headerInfo, ws), col = 1; // imagine indexing from 1
	std::string simHeader[] = { "Absolute difference", "SSIM", "Worst AD", "Worst SSIM" };

	// data
	for (int shader = 0; shader < similarityStats.size(); shader++)
	{
		// data header
		ws.cell(CellRef(row++, 1)).value() = benchmarkedShaderResults[shader].name;
		col = 2;
		for (auto str : simHeader) ws.cell(CellRef(row, col++)).value() = str;
		row++;
		
		for (int prog = 0; prog < shaderProgramTypeCount; prog++)
		{
			col = 1;
			ws.cell(CellRef(row, col++)).value() = shaderProgramTypeNames[prog];
			ws.cell(CellRef(row, col++)).value() = RoundDouble(similarityStats[shader][prog].absoluteDifferencAvg, 0);
			ws.cell(CellRef(row, col++)).value() = RoundDouble(similarityStats[shader][prog].ssimAvg, 3);
			ws.cell(CellRef(row, col++)).value() = similarityStats[shader][prog].worstSimilarity.absoluteDifferenceSum;
			ws.cell(CellRef(row, col++)).value() = RoundDouble(similarityStats[shader][prog].worstSimilarity.ssim, 3);
			row++;
		}
		row++;
	}

	doc.save();
	doc.close();

	Errors::lastError = originalFolder ? Errors::ErrorType::simDone : Errors::ErrorType::simWrongPath;
	Errors::lastErrorMessage = actualSaveLocation.string();
}


#pragma endregion

}




