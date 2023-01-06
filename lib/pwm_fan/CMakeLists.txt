add_library(pwm_fan INTERFACE)

target_include_directories(pwm_fan INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
        )

target_sources(pwm_fan INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/fan.c
        )

target_link_libraries(pwm_fan INTERFACE
        hardware_clocks
        hardware_pwm
        )
