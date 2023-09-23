include(FetchContent)

# set(CXX_STANDARD 20)

# set(CXX_STANDARD_REQUIRED ON)
#-----------------------------------------------------------------------------------------
FetchContent_Declare(
        function2
        GIT_REPOSITORY https://github.com/Naios/function2.git
        GIT_TAG 4.2.2
)
FetchContent_MakeAvailable(function2)
#-----------------------------------------------------------------------------------------
FetchContent_Declare(
        mimalloc
        GIT_REPOSITORY https://github.com/microsoft/mimalloc
        GIT_TAG master
)
FetchContent_MakeAvailable(mimalloc)
#-----------------------------------------------------------------------------------------

get_filename_component(LIB_PATH "." ABSOLUTE)
get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)


file(GLOB_RECURSE LIB_ASM_SOURCES ${LIB_PATH}/*.S)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.hpp)
file(GLOB_RECURSE LIB_CPP ${LIB_PATH}/*.cpp)

enable_language(ASM)

add_library(exe STATIC ${LIB_ASM_SOURCES} ${LIB_HEADERS} ${LIB_CPP})

target_include_directories(exe PUBLIC ${LIB_INCLUDE_PATH})

target_link_libraries(exe PUBLIC function2 mimalloc)
