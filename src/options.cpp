#include "options.h"
#include "logger.h"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <iostream>

using namespace boost::program_options;

std::unordered_map<std::string, std::variant<std::string, int, float,
                                             std::vector<std::string>>>
readCLIOptions(int argc, char *argv[]) {
    std::string inputPDF, outputPDF, imageFile, qrText;
    std::unordered_map<std::string, std::variant<std::string, int, float,
                                                 std::vector<std::string>>>
        cliOption;
    bool invalid_value = false;

    // parse options using boost::program_options
    options_description generic("Generic");
    // clang-format off
    generic.add_options()
        ("help,h", "Produce this help message")
        ("input-file,i", value<std::string>()->required(), "Input file")
        ("output-file,o", value<std::string>()->required(), "Output file")
        ("rotate", value<int>()->default_value(0), "Assume page is rotated by 0/90/180/270 degrees")
        ("debug", "Print extra debug messages");

    options_description imgOptions("Image");
    imgOptions.add_options()
        ("stamp,s", value<std::string>(), "Image to embed")
        ("img-side", value<int>()->default_value(0), "Side of the document: 0 center (default), 1 left, 2 right")
        ("img-scale", value<float>()->default_value(1),"Scale image by a factor eg. 0.5")
        ("img-top-margin", value<float>()->default_value(10),"Set a margin for the image placement from the top of the page")
        ("img-side-margin", value<float>()->default_value(15),"Set a margin for the image placement from the sides of the page");

    options_description qrOptions("QR");
    qrOptions.add_options()
        ("qr", value<std::string>(), "Add QR instead of image using the specified text")
        ("link", "QR value is a URL. Add clickable link")
        ("qr-side", value<int>()->default_value(0), "Side of the document: 0 center (default), 1 left, 2 right")
        ("qr-scale", value<float>()->default_value(1),"Scale QR by a factor eg. 0.5")
        ("qr-top-margin", value<float>()->default_value(10),"Set a margin for the QR placement from the top of the page")
        ("qr-side-margin", value<float>()->default_value(15),"Set a margin for the QR placement from the sides of the page");

    options_description textOptions("Text (only in latin alphabet)");
    textOptions.add_options()
        ("add-text", value<std::vector<std::string>>()->multitoken(), "Add extra text to the first page. It can take multiple "
            "strings of the form '[x,y:][size:][style:]text' where x,y are the coordinates where the text will appear, size "
            "is a float and gives the font size of the text and style can be 'i', 'b', 'bi', 'ib' for italic, bold and bold+italic");

    options_description programOptions("Valid Options");
    programOptions.add(generic).add(imgOptions).add(qrOptions).add(textOptions);
    // clang-format on

    variables_map vm;
    try {
        store(command_line_parser(argc, argv).options(programOptions).run(),
              vm);
        notify(vm);
    } catch (error &e) {
        if (!vm.count("help")) {
            std::cerr << e.what() << std::endl << std::endl;
        }

        std::cout << programOptions << std::endl;
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

    if (vm.count("img-side")) {
        int side;

        side = vm["img-side"].as<int>();
        if (side < 0 || side > 2) {
            invalid_value = true;
        }

        if (invalid_value) {
            std::cerr << "Wrong value for option --side. Valid options are "
                         "0, 1, 2."
                      << std::endl;
            exit(2);
        }

        cliOption["img-side"] = side;
    }

    if (vm.count("qr-side")) {
        int side;

        side = vm["qr-side"].as<int>();
        if (side < 0 || side > 2) {
            invalid_value = true;
        }

        if (invalid_value) {
            std::cerr << "Wrong value for option --side. Valid options are "
                         "0, 1, 2."
                      << std::endl;
            exit(2);
        }

        cliOption["qr-side"] = side;
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

    float img_scale;
    img_scale = vm["img-scale"].as<float>();
    if (img_scale < 0) {
        invalid_value = true;
    }

    if (invalid_value) {
        std::cerr << "Bad value for option img-scale. Valid options are real "
                     "values >0"
                  << std::endl;
        exit(2);
    }

    cliOption["img-scale"] = img_scale;

    float qr_scale;
    qr_scale = vm["qr-scale"].as<float>();
    if (qr_scale < 0) {
        invalid_value = true;
    }

    if (invalid_value) {
        std::cerr
            << "Bad value for option qr-scale. Valid options are real values >0"
            << std::endl;
        exit(2);
    }

    cliOption["qr-scale"] = qr_scale;

    cliOption["img-top-margin"] = vm["img-top-margin"].as<float>();

    cliOption["img-side-margin"] = vm["img-side-margin"].as<float>();

    cliOption["qr-top-margin"] = vm["qr-top-margin"].as<float>();

    cliOption["qr-side-margin"] = vm["qr-side-margin"].as<float>();

    cliOption["text"] = vm.count("add-text")
                            ? vm["add-text"].as<std::vector<std::string>>()
                            : std::vector<std::string>();

    if (vm.count("debug")) {
        logger.setEnabled(true);
    }

    if ((argc < 4) ||
        (!cliOption.contains("qrText") && !cliOption.contains("imageFile") &&
         !cliOption.contains("text")) ||
        vm.empty() || vm.count("help")) {
        std::cout << programOptions << std::endl;
        exit(1);
    }

    return cliOption;
}