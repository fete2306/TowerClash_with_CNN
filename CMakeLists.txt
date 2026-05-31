cmake_minimum_required(VERSION 3.16)
project(TowerClash_with_CNN LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# external 根目录
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# -------------------------
# 第三方：ImGui (构建为静态库)
# -------------------------
add_library(imgui STATIC
	${EXTERNAL_DIR}/imgui/imgui.cpp
	${EXTERNAL_DIR}/imgui/imgui_draw.cpp
	${EXTERNAL_DIR}/imgui/imgui_tables.cpp
	${EXTERNAL_DIR}/imgui/imgui_widgets.cpp
	${EXTERNAL_DIR}/imgui/imgui_demo.cpp
	# 添加 Win32 和 DX11 后端源码
	${EXTERNAL_DIR}/imgui/backends/imgui_impl_win32.cpp
	${EXTERNAL_DIR}/imgui/backends/imgui_impl_dx11.cpp
)
# 同时将 backends 目录加入头文件搜索路径
target_include_directories(imgui PUBLIC 
	${EXTERNAL_DIR}/imgui
	${EXTERNAL_DIR}/imgui/backends
)


# 可选地把后端源码加入到 imgui 库（如果存在）

# -------------------------
# 第三方：nlohmann/json（头文件库）
# -------------------------
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${EXTERNAL_DIR}/json)

# -------------------------
# 第三方：DirectXTK（DirectX Tool Kit）
# -------------------------
set(BUILD_TOOLS OFF CACHE BOOL "Disable building XWBTool" FORCE)
add_subdirectory(${EXTERNAL_DIR}/DirectXTK-may2026)

# -------------------------
# 项目源文件
# -------------------------
file(GLOB_RECURSE GAME_SOURCES
	${CMAKE_SOURCE_DIR}/src/*.cpp
	${CMAKE_SOURCE_DIR}/src/**/*.cpp
)
list(FILTER GAME_SOURCES EXCLUDE REGEX "basic_virtual\\.cpp$")

add_executable(game WIN32 ${GAME_SOURCES})
target_include_directories(game PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/ui
	${EXTERNAL_DIR}/imgui
	${EXTERNAL_DIR}/json
	${EXTERNAL_DIR}/DirectXTK-may2026/Inc
)
target_link_libraries(game PRIVATE imgui nlohmann_json DirectXTK dxgi d3d11 d3dcompiler)
target_compile_features(game PUBLIC cxx_std_20)

# 编译选项
if (MSVC)
	target_compile_options(game PRIVATE /W4 /permissive- /bigobj /utf-8)
else()
	target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS game RUNTIME DESTINATION bin)

# 使用说明提示（可选）
message(STATUS "Configured project: ${PROJECT_NAME}")
message(STATUS "External dir: ${EXTERNAL_DIR}")

