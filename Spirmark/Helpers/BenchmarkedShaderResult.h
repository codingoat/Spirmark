#pragma once

#include <string>
#include <Bencher.h>
#include <Spirver.h>
#include <ShaderProgramType.h>

/// Contains performance data and statistics for all optimization variants of a shader
struct BenchmarkedShaderResult {
	BenchmarkedShaderResult() {};
	BenchmarkedShaderResult(const std::string& name, const std::string& path) : name(name), path(path) {}
	BenchmarkedShaderResult(BenchmarkedShaderResult& o) : name(o.name), path(o.path)
	{
		std::copy(std::begin(o.results), std::end(o.results), results);
		std::copy(std::begin(o.stat), std::end(o.stat), stat);
		usefulSpirvMultiOptIters = o.usefulSpirvMultiOptIters;
	}
	BenchmarkedShaderResult(BenchmarkedShaderResult&& o) : name(std::move(o.name)), path(std::move(o.path))
	{
		std::move(std::begin(o.results), std::end(o.results), results);
		std::move(std::begin(o.stat), std::end(o.stat), stat);
		usefulSpirvMultiOptIters = o.usefulSpirvMultiOptIters;
	}
	BenchmarkedShaderResult& operator=(BenchmarkedShaderResult& o)
	{
		name = o.name;
		path = o.path;
		std::copy(std::begin(o.results), std::end(o.results), results);
		std::copy(std::begin(o.stat), std::end(o.stat), stat);
		usefulSpirvMultiOptIters = o.usefulSpirvMultiOptIters;
		return *this;
	}
	BenchmarkedShaderResult& operator=(BenchmarkedShaderResult&& o)
	{
		name = std::move(o.name);
		path = std::move(o.path);
		std::move(std::begin(o.results), std::end(o.results), results);
		std::move(std::begin(o.stat), std::end(o.stat), stat);
		usefulSpirvMultiOptIters = o.usefulSpirvMultiOptIters;
		return *this;
	}

	std::string name, path;
	BenchmarkResult results[shaderProgramTypeCount];
	Spirver::ShaderStat stat[shaderProgramTypeCount];
	int usefulSpirvMultiOptIters = 0;

	ShaderProgramType GetRecommendedProgramType() const
	{
		auto best = std::min_element(std::begin(results), std::end(results), [](const BenchmarkResult& a, const BenchmarkResult& b) {return a.mean.gpu < b.mean.gpu; });
		if (best->mean.gpu > results[glsl].mean.gpu * 0.95) return ShaderProgramType::glsl; // don't recommend optimization for marginal gains
		else return ShaderProgramType(best - results);
	}
};