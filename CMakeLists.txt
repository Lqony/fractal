cmake_minimum_required(VERSION 2.6)
project(fractal)

# enforce C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost 1.58 QUIET REQUIRED COMPONENTS system filesystem thread date_time chrono regex serialization program_options locale)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(fractal main.cpp)

target_link_libraries(fractal 
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_LIBRARIES}
	cryptopp
)

install(TARGETS fractal RUNTIME DESTINATION bin)
