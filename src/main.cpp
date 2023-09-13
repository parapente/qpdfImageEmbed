#include <Magick++.h>
#include <cmath>
#include <qrencode.h>

#include "config.h"
#include "extraText.h"
#include "logger.h"
#include "options.h"
#include "pdfProcessor.h"
#include "rect.h"

Logger logger;

int main(int argc, char *argv[]) {

    std::unordered_map<std::string, std::variant<std::string, int, float,
                                                 std::vector<std::string>>>
        cliOption;

    cliOption = readCLIOptions(argc, argv);

    Magick::InitializeMagick(nullptr);

    PDFProcessor pdf_processor;
    pdf_processor.open(std::get<std::string>(cliOption["inputPDF"]));

    if (cliOption.contains("rotate")) {
        pdf_processor.rotate(std::get<int>(cliOption["rotate"]));
    }

    int side = 0;
    if (cliOption.contains("side")) {
        side = std::get<int>(cliOption["side"]);
    }
    pdf_processor.setPosition(side);

    if (cliOption.contains("qrText")) {
        logger << "QR Text: " << std::get<std::string>(cliOption["qrText"])
               << "\n";
        QRcode *qr = QRcode_encodeString(
            std::get<std::string>(cliOption["qrText"]).c_str(), 0, QR_ECLEVEL_M,
            QR_MODE_8, 1);

        if (qr == nullptr) {
            logger << "QRcode_encodeString failed!\n";
            exit(3);
        }

        logger << "QR version: " << qr->version << "\n";
        logger << "QR width: " << qr->width << "\n";

        pdf_processor.addImage(new ImageProvider(qr),
                               std::get<float>(cliOption["scale"]),
                               std::get<float>(cliOption["top-margin"]),
                               std::get<float>(cliOption["side-margin"]),
                               std::get<std::string>(cliOption["link"]));
    }

    if (cliOption.contains("imageFile")) {
        pdf_processor.addImage(
            new ImageProvider(
                std::get<std::string>(cliOption["imageFile"]).c_str()),
            std::get<float>(cliOption["scale"]),
            std::get<float>(cliOption["top-margin"]),
            std::get<float>(cliOption["side-margin"]));
    }

    // Add extra text if requested
    const std::vector<std::string> text_vector =
        std::get<std::vector<std::string>>(cliOption["text"]);
    for (auto text : text_vector) {
        ExtraText parsed_text(text);

        if (text != "") {
            pdf_processor.addExtraText(parsed_text.text(), parsed_text.x(),
                                       parsed_text.y(), parsed_text.font_size(),
                                       "Helvetica", parsed_text.style());
        }
    }

    pdf_processor.save(std::get<std::string>(cliOption["outputPDF"]));

    Magick::TerminateMagick();

    return 0;
}
