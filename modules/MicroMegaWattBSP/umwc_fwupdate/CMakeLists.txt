cmake_minimum_required(VERSION 3.10)

# set the project name
project(umwc_fwupdate VERSION 0.1)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(umwc_fwupdate main.cpp)
target_include_directories(umwc_fwupdate PUBLIC "${PROJECT_BINARY_DIR}" PUBLIC "../umwc_comms/nanopb" PUBLIC "../umwc_comms/protobuf" PUBLIC "../umwc_comms")
target_link_libraries(umwc_fwupdate PRIVATE Pal::Sigslot Threads::Threads umwc_comms everest::framework everest::gpio)

install(TARGETS umwc_fwupdate
        DESTINATION ${EVEREST_MOD_YETIDRIVER_DESTINATION})


