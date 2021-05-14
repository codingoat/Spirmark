cmake_minimum_required(VERSION 3.12)

project(Spirmark VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(DISTRIBUTE_GLEWINFO "Distribute glewinfo.exe and visualinfo.exe with ${PROJECT_NAME}" ON)

add_subdirectory(Spirmark/Spirver)

list(APPEND DEPENDENCY_INCLUDE_FOLDERS
        "${PROJECT_SOURCE_DIR}/dependencies/glsl-opt/include"
        "${PROJECT_SOURCE_DIR}/dependencies/glslang/include"
        "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/include"
        "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/include/imgui"
        "${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/include"
        "${PROJECT_SOURCE_DIR}/dependencies/spirv-cross/include"
        )

if(NOT WIN32)
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})
        find_package(GLEW 2.0 REQUIRED)
        find_package(OpenGL)
ENDIF()


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


list(APPEND SPIRMARK_CPP
        Spirmark/main.cpp
        Spirmark/SpirmarkApp.cpp
        Spirmark/SpirmarkApp.h
        Spirmark/SpirmarkCore.cpp
        Spirmark/SpirmarkCore.h
        Spirmark/SpirmarkUI.cpp
        Spirmark/SpirmarkUI.h
        Spirmark/resource.rc
        )

list(APPEND IMGUI_CPP
        dependencies/OGLPack/include/imgui/imgui.cpp
        dependencies/OGLPack/include/imgui/imgui_demo.cpp
        dependencies/OGLPack/include/imgui/imgui_draw.cpp
        dependencies/OGLPack/include/imgui/imgui_impl_opengl3.cpp
        dependencies/OGLPack/include/imgui/imgui_impl_sdl.cpp
        dependencies/OGLPack/include/imgui/imgui_tables.cpp
        dependencies/OGLPack/include/imgui/imgui_widgets.cpp
        dependencies/OGLPack/include/imgui/ImFileDialog.cpp
        dependencies/OGLPack/include/imgui/imconfig.h
        dependencies/OGLPack/include/imgui/ImFileDialog.h
        dependencies/OGLPack/include/imgui/imgui.h
        dependencies/OGLPack/include/imgui/imgui_impl_opengl3.h
        dependencies/OGLPack/include/imgui/imgui_impl_sdl.h
        dependencies/OGLPack/include/imgui/imgui_internal.h
        dependencies/OGLPack/include/imgui/imstb_rectpack.h
        dependencies/OGLPack/include/imgui/imstb_textedit.h
        dependencies/OGLPack/include/imgui/imstb_truetype.h
        dependencies/OGLPack/include/imgui/stb_image.h
        dependencies/OGLPack/include/imgui/stb_image_write.h
        dependencies/OGLPack/include/imgui/imgui_plot.h
        dependencies/OGLPack/include/imgui/imgui_plot.cpp
        )

list(APPEND HELPERS_CPP
        Spirmark/Helpers/ProgramObject.cpp
        Spirmark/Helpers/ProgramObject.h
        Spirmark/Helpers/ShaderObject.cpp
        Spirmark/Helpers/ShaderObject.h
        Spirmark/Helpers/Bencher.cpp
        Spirmark/Helpers/Bencher.h
        Spirmark/Helpers/ShaderToySource.h
        Spirmark/Helpers/BenchmarkedShaderResult.h
        Spirmark/Helpers/ShaderProgramType.h
        Spirmark/Helpers/UIWidgets.cpp
        Spirmark/Helpers/UIWidgets.h
        Spirmark/Helpers/Mesh_OGL3.cpp
        Spirmark/Helpers/Mesh_OGL3.h
        Spirmark/Helpers/ObjParser_OGL3.cpp
        Spirmark/Helpers/ObjParser_OGL3.h
        Spirmark/Helpers/StructuralSimilarity.cpp
        Spirmark/Helpers/StructuralSimilarity.h
        Spirmark/Helpers/UniformValues.h
        Spirmark/Helpers/IO.h
        Spirmark/Helpers/IO.cpp
        Spirmark/Helpers/Errors.h
        Spirmark/Helpers/Errors.cpp
        Spirmark/Helpers/BenchmarkedShader.h
        Spirmark/Helpers/BenchmarkedShader.cpp
        Spirmark/Helpers/BenchmarkMode.h
        Spirmark/Helpers/BenchmarkMode.cpp
        )

add_executable(${PROJECT_NAME}
        ${SPIRMARK_CPP}
        ${IMGUI_CPP}
        ${HELPERS_CPP}
)

source_group("Spirmark" FILES ${SPIRMARK_CPP})
source_group("imgui" FILES ${IMGUI_CPP})
source_group("Helpers" FILES ${HELPERS_CPP})


target_include_directories(${PROJECT_NAME} PUBLIC
        "/usr/include"
        "/usr/include/SDL2"
        "${PROJECT_SOURCE_DIR}/Spirmark"
        "${PROJECT_SOURCE_DIR}/Spirmark/Helpers"
        "${DEPENDENCY_INCLUDE_FOLDERS}"
        "${OPENGL_INCLUDE_DIRS}"
        )

list(APPEND SPIRMARK_LIB_DIRS ${PROJECT_SOURCE_DIR}/dependencies/OGLPack/lib/)

IF(WIN32)
        list(APPEND SPIRMARK_LIB_DIRS
                ${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/lib/
        )
ELSE()
        list(APPEND SPIRMARK_LIB_DIRS
                debug ${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/lib/debug/
                optimized ${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/lib/release/
        )
ENDIF()

target_link_directories(${PROJECT_NAME} PUBLIC ${SPIRMARK_LIB_DIRS})


LIST(APPEND SPIRMARK_LIBS Spirver)

if(WIN32)
        LIST(APPEND SPIRMARK_LIBS
                SDL2
                SDL2main
                SDL2_image
                glew32
                opengl32
                OpenXLSX-shared
        )
ELSE()
        LIST(APPEND SPIRMARK_LIBS
                ${SDL2_LIBRARIES}
                SDL2_image
                ${OPENGL_LIBRARIES}
                GLEW::GLEW
                OpenXLSX-static
        )
ENDIF()

target_link_libraries(${PROJECT_NAME} ${SPIRMARK_LIBS})



set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/_output")
file(COPY "${PROJECT_SOURCE_DIR}/Assets" DESTINATION "${CMAKE_BINARY_DIR}/_output")
file(COPY "${PROJECT_SOURCE_DIR}/Results" DESTINATION "${CMAKE_BINARY_DIR}/_output")
file(COPY "${PROJECT_SOURCE_DIR}/Shaders" DESTINATION "${CMAKE_BINARY_DIR}/_output")
file(COPY "${PROJECT_SOURCE_DIR}/Similarity" DESTINATION "${CMAKE_BINARY_DIR}/_output")
configure_file("${PROJECT_SOURCE_DIR}/imgui_default.ini" "${CMAKE_BINARY_DIR}/_output/imgui.ini" COPYONLY)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/_output/Exports")

if(WIN32)
        file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/glew32.dll" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/SDL2.dll" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/SDL2_image.dll" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/libpng16-16.dll" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/zlib1.dll" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/bin/OpenXLSX-shared.dll" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        
        if(DISTRIBUTE_GLEWINFO)
                file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/glewinfo.exe" DESTINATION "${CMAKE_BINARY_DIR}/_output")
                file(COPY "${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/visualinfo.exe" DESTINATION "${CMAKE_BINARY_DIR}/_output")
        endif()
else()
	file(COPY "${PROJECT_SOURCE_DIR}/Spirmark.sh" DESTINATION "${CMAKE_BINARY_DIR}/_output")
endif()

# visual studio debugging environment

file(WRITE "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.Cpp.user.props.in" [=[
<?xml version="1.0" encoding="utf-8"?> 
<Project DefaultTargets="Build" ToolsVersion="15.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LocalDebuggerEnvironment>PATH=${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/;${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/bin/;%PATH%</LocalDebuggerEnvironment>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LocalDebuggerEnvironment>PATH=${PROJECT_SOURCE_DIR}/dependencies/OGLPack/bin/;${PROJECT_SOURCE_DIR}/dependencies/OpenXLSX/bin/;%PATH%</LocalDebuggerEnvironment>
  </PropertyGroup>
</Project>
]=])

configure_file("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.Cpp.user.props.in" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.Cpp.user.props")

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        VS_USER_PROPS "${PROJECT_NAME}.Cpp.user.props"
) 

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Spirmark)
set_property(TARGET Spirmark PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/_output")

# include(InstallRequiredSystemLibraries)
# set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/License.txt")
# set(CPACK_PACKAGE_VERSION_MAJOR "${Spirmark_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${Spirmark_VERSION_MINOR}")
# include(CPack)

# install(TARGETS ${PROJECT_NAME} DESTINATION bin)
