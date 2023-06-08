configure_file(config.h.in config.h)

find_package(Qt5 REQUIRED COMPONENTS Core Gui)
find_package(qpdf)
find_package(boost_program_options)

add_executable(qpdfImageEmbed main.cpp imageProvider.cpp options.cpp)

target_include_directories(qpdfImageEmbed PUBLIC "${PROJECT_BINARY_DIR}/src")
target_link_libraries(qpdfImageEmbed PRIVATE Qt5::Core Qt5::Gui qpdf boost_program_options)