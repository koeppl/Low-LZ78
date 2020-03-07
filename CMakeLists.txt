cmake_minimum_required(VERSION 3.5)
project(cdslib)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

set(USE_MALLOC_COUNT OFF CACHE BOOL "Display the amount of allocated memory! Needs the malloc_count submodule.")
set(MALLOC_COUNT_SOURCE "")
set(MALLOC_COUNT_LIBS "")

if(USE_MALLOC_COUNT)
	include_directories(external/malloc_count)
	set(MALLOC_COUNT_SOURCE "external/malloc_count/malloc_count.c")
	set(MALLOC_COUNT_LIBS "-ldl")
endif(USE_MALLOC_COUNT)


set(CXX_STANDARD gnu++17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -march=native -fdiagnostics-color=auto -std=${CXX_STANDARD} -DPRINT_STATS") # -DMALLOC_DISABLED=1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -ggdb -DDEBUG") #-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native -march=native -DNDEBUG")

# Global dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules")
include(git_submodule_subdirectories)
include(find_or_download_package)
#git_submodule_subdirectory(external/broadwordsearch)
git_submodule_subdirectory(external/bit_span)
git_submodule_subdirectory(external/compact_sparse_hash)
#git_submodule_subdirectory(external/separate_chaining)
include_directories(external/bit_span/include)
include_directories(external/compact_sparse_hash/include/)
include_directories(external/separate_chaining/include)
include_directories(external/broadwordsearch/include/)

find_or_download_package(Glog GLOG glog)

#change for yours sdsl library location
# include_directories(/home/rcanovas/include)
# link_directories(/home/rcanovas/lib)

link_libraries(sdsl divsufsort divsufsort64 glog)


set(SOURCE_FILES tests/CompressText.cpp)
set(SOURCE_FILES2 tests/DecompressHLZ.cpp)

add_executable(CompressText ${SOURCE_FILES} ${MALLOC_COUNT_SOURCE})
add_executable(DecompressHLZ ${SOURCE_FILES2} ${MALLOC_COUNT_SOURCE})
target_link_libraries(CompressText ${MALLOC_COUNT_LIBS})
target_link_libraries(DecompressHLZ ${MALLOC_COUNT_LIBS})

MESSAGE( STATUS "Built Type: " ${CMAKE_BUILD_TYPE} )
MESSAGE( STATUS "CXX Flags: " ${CMAKE_CXX_FLAGS} )

