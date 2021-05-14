#pragma once

namespace Spirmark
{
    enum BenchmarkMode { SingleFullscreen = 0, SingleModel = 1, MultiModel = 2, Similarity = 3, benchmarkModeCount = 4 };
    extern std::string benchmarkModeNames[benchmarkModeCount];
    extern std::string benchmarkModeIcons[benchmarkModeCount];
}