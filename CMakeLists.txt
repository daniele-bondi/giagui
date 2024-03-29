cmake_minimum_required(VERSION 3.13)

project(giagui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable")

include_directories(source)

option(ENABLE_ASSERT "Enable additional assertions" OFF)
if(ENABLE_ASSERT)
	add_compile_definitions(ENABLE_ASSERT=1)
endif()

option(ENABLE_DEBUG_DRAW_GEOBOUNDARY_VERTICES "Draw points on the vertices of the currently selected polygon" OFF)
if(ENABLE_DEBUG_DRAW_GEOBOUNDARY_VERTICES)
	add_compile_definitions(ENABLE_DEBUG_DRAW_GEOBOUNDARY_VERTICES=1)
endif()

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x051200) # We don't want old APIs
#add_compile_definitions(QT_NO_CAST_FROM_ASCII) # Disable ascii strings in qt API
#add_compile_definitions(QT_NO_CAST_TO_ASCII)   # Disable ascii strings in qt API

find_package(h3 REQUIRED)

find_package(cpptoml REQUIRED)

### Qt stuff ###
set(CMAKE_INCLUDE_CURRENT_DIR ON) # Find includes in corresponding build directories
set(CMAKE_AUTOMOC ON) # Instruct CMake to run the moc automatically when needed
set(CMAKE_AUTOUIC ON) # Create code from a list of Qt designer ui files
set(CMAKE_AUTORCC ON) # Run resource compiler
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Svg     REQUIRED)

set(SOURCE_FILES
    source/main.cpp
    source/Containers.hpp
    source/GeoValue.hpp
    source/Dataset.cpp source/Dataset.hpp
    source/SimulationConfig.hpp source/SimulationConfig.cpp
    source/MapUtils.hpp
    source/models/DatasetListModel.cpp source/models/DatasetListModel.hpp
    source/MapWindow.cpp source/MapWindow.hpp
    source/MapView.cpp source/MapView.hpp
    source/GeoValueValidator.cpp source/GeoValueValidator.hpp
    source/DatasetListWidget.cpp source/DatasetListWidget.hpp
    source/DatasetControlWidget.cpp source/DatasetControlWidget.hpp
    source/dialogs/SimulationConfigDialog.cpp source/dialogs/SimulationConfigDialog.hpp
    source/dialogs/DatasetCreateDialog.cpp source/dialogs/DatasetCreateDialog.hpp
    source/preprocess/H3Map.cpp source/preprocess/H3Map.hpp
    source/preprocess/Project.cpp source/preprocess/Project.hpp)

set(RESOURCE_FILES
	resources/resources.qrc)

set(UI_FILES
    )

add_executable(${PROJECT_NAME}
	${SOURCE_FILES}
	${UI_FILES}
	${RESOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
	h3::h3
	cpptoml
	Qt5::Widgets
	Qt5::Svg)
