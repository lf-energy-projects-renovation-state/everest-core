set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/modules/Auth/include")

add_library(auth_handler STATIC)
ev_register_library_target(auth_handler)

target_sources(auth_handler PRIVATE
    AuthHandler.cpp
    Connector.cpp
    ReservationHandler.cpp
    ConnectorStateMachine.cpp
)

get_target_property(GENERATED_INCLUDE_DIR generate_cpp_files EVEREST_GENERATED_INCLUDE_DIR)

target_include_directories(auth_handler PRIVATE
    ${INCLUDE_DIR}
    ${GENERATED_INCLUDE_DIR}
)

add_dependencies(auth_handler generate_cpp_files)

target_link_libraries(auth_handler
PRIVATE
    everest::timer
    date::date
    date::date-tz
    everest::framework
    everest::staging::helpers
)

# needs c++ 14
target_compile_features(auth_handler PRIVATE cxx_std_14)

if(EVEREST_ENABLE_COMPILE_WARNINGS)
    target_compile_options(auth_handler
        PRIVATE ${EVEREST_COMPILE_OPTIONS}
    )
endif()
