project(DDexchange)

set(SOURCES
    dd_exchange.hpp
    dd_exchange.cpp
    sdd_sat_enum.hpp
)
set(ENV{LD_LIBRARY_PATH} "/usr/local/lib")
set(CMAKE_CXX_FLAGS "-fconcepts -std=c++17")

add_executable(peg-solitaire peg-solitaire.cpp ${SOURCES})
target_link_libraries(peg-solitaire sylvan sdd)

add_executable(test test.cpp ${SOURCES})
target_link_libraries(test sylvan sdd)
