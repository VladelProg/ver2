cmake_minimum_required(VERSION 3.15)
project(embedded_cli VERSION 1.0.0 LANGUAGES C)
# Добавьте после project()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_BUILD_TYPE Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Детект платформы
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM "linux")
    add_compile_definitions(LINUX=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    set(PLATFORM "stm32") 
    add_compile_definitions(STM32=1)
else()
    set(PLATFORM "generic")
endif()

# Библиотека
add_library(embedded_cli STATIC
    src/cli.c
    src/cli_commands.c
    src/platform/${PLATFORM}/${PLATFORM}_io.c
    src/platform/${PLATFORM}/${PLATFORM}_bg_tasks.c
)

# Include директории
target_include_directories(embedded_cli PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Примеры
add_executable(linux_example examples/linux_example.c)
target_link_libraries(linux_example embedded_cli)
