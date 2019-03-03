cmake_minimum_required(VERSION 3.5)
project(cdslib)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Global dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules")
include(git_submodule_subdirectories)
include(find_or_download_package)
git_submodule_subdirectory(external/bit_span)
git_submodule_subdirectory(external/compact_sparse_hash)
#git_submodule_subdirectory(external/separate_chaining)
include_directories(external/bit_span/include)
include_directories(external/compact_sparse_hash/include/)
include_directories(external/separate_chaining/include)

find_or_download_package(Glog GLOG glog)

#change for yours sdsl library location
include_directories(/home/rcanovas/include)
link_directories(/home/rcanovas/lib)

link_libraries(sdsl divsufsort divsufsort64 glog)


set(SOURCE_FILES tests/CompressText.cpp)
set(SOURCE_FILES2 tests/DecompressHLZ.cpp)

add_executable(CompressText ${SOURCE_FILES})
add_executable(DecompressHLZ ${SOURCE_FILES2})
