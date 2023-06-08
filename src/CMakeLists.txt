configure_file(config.h.in config.h)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(qpdf REQUIRED)
find_package(boost_program_options REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS Magick++)

add_executable(qpdfImageEmbed main.cpp imageProvider.cpp options.cpp)

target_include_directories(qpdfImageEmbed PUBLIC "${PROJECT_BINARY_DIR}/src" "${ImageMagick_Magick++_INCLUDE_DIRS}")
target_link_libraries(qpdfImageEmbed PRIVATE Qt5::Core qpdf::libqpdf boost_program_options "${ImageMagick_Magick++_LIBRARY}")