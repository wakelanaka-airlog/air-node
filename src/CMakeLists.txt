# Hardware-agnostic sensor_node logic, built for the host so it can be
# exercised by the GoogleTest suite in tests/. ESP32-specific code (under
# platform/esp32/) depends on ESP-IDF headers and is excluded here — it is
# compiled instead as part of the firmware/ ESP-IDF project.

file(GLOB_RECURSE SENSOR_NODE_SOURCES CONFIGURE_DEPENDS "*.cpp")
list(FILTER SENSOR_NODE_SOURCES EXCLUDE REGEX "/platform/esp32/")

if(SENSOR_NODE_SOURCES)
    add_library(sensor_node ${SENSOR_NODE_SOURCES})
    target_include_directories(sensor_node PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/state_machine
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    )
else()
    # No production sources yet.
    add_library(sensor_node INTERFACE
            logging/FilteringLogger.cpp
            logging/FilteringLogger.h)
    target_include_directories(sensor_node INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/state_machine
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    )
endif()
