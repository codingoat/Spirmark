#include <string>
#include "BenchmarkMode.h"
#include <IconsFontAwesome5.h>

namespace Spirmark
{
    std::string benchmarkModeNames[benchmarkModeCount] = {"Single Shader, Fullscreen", "Single Shader, Model", "Multiple Shaders, Model", "Visual Similarity"};
    std::string benchmarkModeIcons[benchmarkModeCount] = { ICON_FA_EXPAND, ICON_FA_CUBE, ICON_FA_CUBES, ICON_FA_EYE };
}