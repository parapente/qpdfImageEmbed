#include "options.h"
#include "logger.h"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <iostream>

using namespace boost::program_options;

std::unordered_map<std::string, std::variant<std::string, int, float>>
readCLIOptions(int argc, char *argv[]) {
    std::string inputPDF, outputPDF, imageFile, qrText;
    std::unordered_map<std::string, std::variant<std::string, int, float>>
        cliOption;
    bool invalid_value = false;

    // parse options using boost::program_options
    options_description desc("Allowed options:");
    // clang-format off
    desc.add_options()
        ("help,h", "Produce this help message")
        ("input-file,i", value<std::string>()->required(), "Input file")
        ("stamp,s", value<std::string>(), "Image to embed")
        ("output-file,o", value<std::string>()->required(), "Output file")
        ("side", value<int>()->default_value(0), "Side of the document: 0 center (default), 1 left, 2 right")
        ("rotate", value<int>()->default_value(0), "Assume page is rotated by 0/90/180/270 degrees")
        ("qr", value<std::string>(), "Add QR instead of image using the specified text")
        ("link", "QR value is a URL. Add clickable link")
        ("scale", value<float>()->default_value(1),"Scale image by a factor eg. 0.5")
        ("top-margin", value<float>()->default_value(10),"Set a margin for the image placement from the top of the page")
        ("side-margin", value<float>()->default_value(15),"Set a margin for the image placement from the sides of the page")
        ("debug", "Print extra debug messages");
    // clang-format on

    variables_map vm;
    try {
        store(command_line_parser(argc, argv).options(desc).run(), vm);
        notify(vm);
    } catch (error &e) {
        if (!vm.count("help")) {
            std::cerr << e.what() << std::endl << std::endl;
        }

        std::cout << desc << std::endl;
        exit(1);
    }

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
        int side;

        side = vm["side"].as<int>();
        if (side < 0 || side > 2) {
            invalid_value = true;
        }

        if (invalid_value) {
            std::cerr << "Wrong value for option --side. Valid options are "
                         "0, 1, 2."
                      << std::endl;
            exit(2);
        }

        cliOption["side"] = side;
    }

    if (vm.count("rotate")) {
        int assumeRotate;

        assumeRotate = vm["rotate"].as<int>();

        if ((assumeRotate != 0) && (assumeRotate != 90) &&
            (assumeRotate != 180) && (assumeRotate != 270)) {
            invalid_value = true;
        }

        if (invalid_value) {
            std::cerr << "Valid rotate values are 0, 90, 180 or 270 only."
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

    float scale;
    scale = vm["scale"].as<float>();
    if (scale < 0) {
        invalid_value = true;
    }

    if (invalid_value) {
        std::cerr
            << "Bad value for option scale. Valid options are real values >0"
            << std::endl;
        exit(2);
    }

    cliOption["scale"] = scale;

    cliOption["top-margin"] = vm["top-margin"].as<float>();

    cliOption["side-margin"] = vm["side-margin"].as<float>();

    if (vm.count("debug")) {
        logger.setEnabled(true);
    }

    // We cannot (yet) embed an image and add a qr in the same operation
    if ((cliOption.contains("qrText") && cliOption.contains("imageFile")) ||
        (argc < 4) ||
        (!cliOption.contains("qrText") && !cliOption.contains("imageFile")) ||
        vm.empty() || vm.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }

    return cliOption;
}