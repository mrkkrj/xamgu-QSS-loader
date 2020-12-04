if(UNIX)
    cmake_minimum_required(VERSION 3.10)
else()
    cmake_minimum_required(VERSION 3.13)
endif()
 
#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
project(QssLoaderDemoApp VERSION 1.0.0 LANGUAGES CXX)
 
################################################################################
# Use solution folders feature
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
 
################################################################################
# Use Qt
################################################################################
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
 
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

################################################################################
# Source groups
################################################################################
set(Form_Files
    "Demo/QssLoaderDemoApp.ui"
)
source_group("Form Files" FILES ${Form_Files})
 
set(Resource_Files
    "Demo/QssLoaderDemoApp.qrc"
)
source_group("Resource Files" FILES ${Resource_Files})

set(Header_Files
	"Demo/QssLoaderDemoApp.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files_Loader
    "Source/QsseFileLoader.cpp"
    "Source/QsseFileLoader.h"
)
source_group("Source Files\\loader" FILES ${Source_Files_Loader})

set(Source_Files
    "Demo/QssLoaderDemoApp.cpp"
    "Demo/main.cpp"
)
source_group("Source Files" FILES ${Source_Files})


set(ALL_FILES
    ${Form_Files}
    ${Resource_Files}  	
    ${Header_Files}
    ${Source_Files_Loader}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
if(MSVC)
    # set /SUBSYSTEM:WINDOWS
    add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})
else()
    add_executable(${PROJECT_NAME} ${ALL_FILES})
endif()
 
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.;"
)
 
################################################################################
# Compiler flags
################################################################################
if(MSVC)
    # IF needed: quiet warnings related to fopen, sscanf, etc.
    #target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
 
# force more good warnings!
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall>
    $<$<CXX_COMPILER_ID:MSVC>:
       /W4>)

if(MSVC)
    # avoid warning D9025 - /W4 overriding /W3 warning level!!!
    string(REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()
 
################################################################################
# Dependencies
################################################################################
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "Qt5::Core;"
    "Qt5::Gui;"
    "Qt5::Widgets;"
)

target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
