add_library(bme280 INTERFACE)

target_include_directories(bme280 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
        )

target_sources(bme280 INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/bme280.c
        )

target_link_libraries(bme280 INTERFACE
        hardware_spi
        )
