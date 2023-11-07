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

    int qr_side = 0;
    if (cliOption.contains("qr-side")) {
        qr_side = std::get<int>(cliOption["qr-side"]);
    }
    pdf_processor.setPosition(qr_side);

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
                               std::get<float>(cliOption["qr-scale"]),
                               std::get<float>(cliOption["qr-top-margin"]),
                               std::get<float>(cliOption["qr-side-margin"]),
                               std::get<std::string>(cliOption["link"]));
    }

    int img_side = 0;
    if (cliOption.contains("img-side")) {
        img_side = std::get<int>(cliOption["img-side"]);
    }
    pdf_processor.setPosition(img_side);

    if (cliOption.contains("imageFile")) {
        if (cliOption.contains("img-x")) {
            Point p(std::get<float>(cliOption["img-x"]),
                    std::get<float>(cliOption["img-y"]));
            pdf_processor.addImage(
                new ImageProvider(
                    std::get<std::string>(cliOption["imageFile"]).c_str()),
                std::get<float>(cliOption["img-scale"]), 0, 0,
                std::get<std::string>(cliOption["img-link-to"]), &p);
        } else {
            pdf_processor.addImage(
                new ImageProvider(
                    std::get<std::string>(cliOption["imageFile"]).c_str()),
                std::get<float>(cliOption["img-scale"]),
                std::get<float>(cliOption["img-top-margin"]),
                std::get<float>(cliOption["img-side-margin"]),
                std::get<std::string>(cliOption["img-link-to"]));
        }
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
