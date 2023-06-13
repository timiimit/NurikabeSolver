# CMakeList.txt : CMake project for NurikabeSolver, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (NurikabeSolver "NurikabeRules.cpp" "NurikabeRules.h" "Main.cpp" "NurikabeBoard.h" "NurikabeBoard.cpp" "NurikabeSquare.h" "NurikabeSquare.cpp" "Nurikabe.h" "NurikabeSolver.h" "NurikabeSolver.cpp" "Point.h" "Point.cpp" "NurikabeRegion.h" "NurikabeRegion.cpp")
file(COPY "5x5-easy.txt" "7x7-hard.txt" "10x18-medium.txt" "10x18-hard.txt" "10x18-3.txt" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NurikabeSolver PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
