cmake_minimum_required (VERSION 3.8)

add_executable(NurikabeSolver
	"Point.h" "Point.cpp"
	"NurikabeSquare.h" "NurikabeSquare.cpp"
	"NurikabeBoard.h" "NurikabeBoard.cpp"
	"NurikabeRegion.h" "NurikabeRegion.cpp"
	"NurikabeRules.cpp" "NurikabeRules.h"
	"NurikabeSolver.h" "NurikabeSolver.cpp" "NurikabeSolverRules.cpp"
	"Nurikabe.h"
	"Main.cpp"
)
file(
	COPY
		"5x5-easy.txt"
		"7x7-hard.txt"

		"10x10-1.txt"
		"10x10-2.txt"
		"10x10-2-2.txt"
		"10x10-3.txt"
		"10x10-4.txt"
		"10x10-5.txt"

		"10x18-1.txt"
		"10x18-2.txt"
		"10x18-3.txt"
		"10x18-4.txt"

		"14x24-1.txt"
		"14x24-2.txt"
		"14x24-3.txt"

		"16x30-1.txt"

	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NurikabeSolver PROPERTY CXX_STANDARD 20)
endif()

include(CTest)

add_test(NAME 5x5-easy COMMAND NurikabeSolver -i 100 -f 5x5-easy.txt)
add_test(NAME 7x7-hard COMMAND NurikabeSolver -i 1000 -f 7x7-hard.txt)

add_test(NAME 10x10-1 COMMAND NurikabeSolver -i 100 -f 10x10-1.txt)
add_test(NAME 10x10-2 COMMAND NurikabeSolver -i 100 -f 10x10-2.txt)
add_test(NAME 10x10-2-2 COMMAND NurikabeSolver -i 100 -f 10x10-2-2.txt)
add_test(NAME 10x10-3 COMMAND NurikabeSolver -i 100 -f 10x10-3.txt)
add_test(NAME 10x10-4 COMMAND NurikabeSolver -i 100 -f 10x10-4.txt)
add_test(NAME 10x10-5 COMMAND NurikabeSolver -i 1000 -f 10x10-5.txt)

add_test(NAME 10x18-1 COMMAND NurikabeSolver -f 10x18-1.txt)
add_test(NAME 10x18-2 COMMAND NurikabeSolver -f 10x18-2.txt)
add_test(NAME 10x18-3 COMMAND NurikabeSolver -f 10x18-3.txt)
add_test(NAME 10x18-4 COMMAND NurikabeSolver -f 10x18-4.txt)

add_test(NAME 14x24-1 COMMAND NurikabeSolver -f 14x24-1.txt)
add_test(NAME 14x24-2 COMMAND NurikabeSolver -f 14x24-2.txt)
add_test(NAME 14x24-3 COMMAND NurikabeSolver -f 14x24-3.txt)

add_test(NAME 16x30-1 COMMAND NurikabeSolver -f 16x30-1.txt)
