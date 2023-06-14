configure_file(config.h.in config.h)

find_package(qpdf REQUIRED)
find_package(boost_program_options REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS Magick++)

add_compile_definitions(MAGICKCORE_QUANTUM_DEPTH=16 MAGICKCORE_HDRI_ENABLE=0 POINTERHOLDER_TRANSITION=4)

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-Os")

add_executable(qpdfImageEmbed main.cpp imageProvider.cpp options.cpp rect.cpp logger.cpp pdfProcessor.cpp)

target_include_directories(qpdfImageEmbed PRIVATE "${PROJECT_BINARY_DIR}/src" "${ImageMagick_Magick++_INCLUDE_DIRS}")
target_link_libraries(qpdfImageEmbed PRIVATE qpdf::libqpdf boost_program_options "${ImageMagick_Magick++_LIBRARY}" qrencode)

install(TARGETS qpdfImageEmbed DESTINATION bin)