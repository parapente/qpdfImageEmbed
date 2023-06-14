#include "options.h"
#include "logger.h"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <iostream>

using namespace boost::program_options;

std::unordered_map<std::string, std::variant<std::string, int, float>>
readCLIOptions(int argc, char *argv[]) {
    std::string inputPDF, outputPDF, imageFile, qrText;
    int assumeRotate = -1;
    std::unordered_map<std::string, std::variant<std::string, int, float>>
        cliOption;

    // parse options using boost::program_options
    options_description desc("Allowed options:");
    // clang-format off
  desc.add_options()
    ("help,h", "Produce this help message")
    ("input-file,i", value<std::string>(), "Input file")
    ("stamp,s", value<std::string>(), "Image to embed")
    ("output-file,o", value<std::string>(), "Output file")
    ("side", value<int>(), "Side of the document: 0 center (default), 1 left, 2 right")
    ("rotate", value<int>(), "Assume page is rotated by 0/90/180/270 degrees")
    ("qr", value<std::string>(), "Add QR instead of image using the specified text")
    ("link", "QR value is a link")
    ("scale", value<float>(),"Scale image by a factor")
    ("top-margin", value<float>(),"Set a margin for the image placement from the top of the page")
    ("side-margin", value<float>(),"Set a margin for the image placement from the sides of the page")
    ("debug", "Print extra debug messages");
    // clang-format on

    variables_map vm;
    store(command_line_parser(argc, argv).options(desc).run(), vm);
    notify(vm);

    if (vm.count("input-file")) {
        cliOption["inputPDF"] = vm["input-file"].as<std::string>();
    }

    if (vm.count("output-file")) {
        cliOption["outputPDF"] = vm["output-file"].as<std::string>();
    }

    if (vm.count("stamp")) {
        cliOption["imageFile"] = vm["stamp"].as<std::string>();
    }

    if (vm.count("side")) {
        cliOption["side"] = vm["side"].as<int>();
    }

    if (vm.count("rotate")) {
        assumeRotate = vm["rotate"].as<int>();
        if ((assumeRotate != 0) && (assumeRotate != 90) &&
            (assumeRotate != 180) && (assumeRotate != 270)) {
            std::cerr << "Valid rotate values are 0,90,180 or 270 only."
                      << std::endl;
            exit(2);
        }

        cliOption["rotate"] = assumeRotate;
    }

    if (vm.count("qr")) {
        qrText = vm["qr"].as<std::string>();
        cliOption["qrText"] = qrText;

        if (vm.count("link"))
            cliOption["link"] = qrText;
        else
            cliOption["link"] = std::string();
    }

    cliOption["scale"] = 1.0f;
    if (vm.count("scale")) {
        cliOption["scale"] = vm["scale"].as<float>();
    }

    cliOption["top-margin"] = 10.0f;
    if (vm.count("top-margin")) {
        cliOption["top-margin"] = vm["top-margin"].as<float>();
    }

    cliOption["side-margin"] = 15.0f;
    if (vm.count("side-margin")) {
        cliOption["side-margin"] = vm["side-margin"].as<float>();
    }

    if (vm.count("debug")) {
        logger.setEnabled(true);
    }

    if ((cliOption.contains("qrText") && cliOption.contains("imageFile")) ||
        (argc < 4) ||
        (!cliOption.contains("qrText") && !cliOption.contains("imageFile")) ||
        vm.empty() || vm.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }

    return cliOption;
}