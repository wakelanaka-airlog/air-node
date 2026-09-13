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
