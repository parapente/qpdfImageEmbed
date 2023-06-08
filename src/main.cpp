#include <cmath>
#include <qrencode.h>

#include "config.h"
#include "logger.h"
#include "options.h"
#include "pdfProcessor.h"
#include "rect.h"

Logger logger;

int main(int argc, char *argv[]) {

    std::unordered_map<std::string, std::variant<std::string, int>> cliOption;

    cliOption = readCLIOptions(argc, argv);

    PDFProcessor processor;
    processor.open(std::get<std::string>(cliOption["inputPDF"]));

    if (cliOption.contains("rotate")) {
        processor.rotate(std::get<int>(cliOption["rotate"]));
    }

    int side = -1;
    if (cliOption.contains("side")) {
        side = std::get<int>(cliOption["side"]);
    }
    processor.setPosition(side);

    processor.addImage(std::get<std::string>(cliOption["imageFile"]));

    processor.save(std::get<std::string>(cliOption["outputPDF"]));

    return 0;
}
