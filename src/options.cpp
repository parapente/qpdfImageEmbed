#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <iostream>

using namespace std;
using namespace boost::program_options;

QHash<QString, QVariant> readCLIOptions(int argc, char *argv[]) {
  string inputPDF, outputPDF, imageFile, qrText;
  int side = 0, assumeRotate = -1;
  QHash<QString, QVariant> cliOption;

  // parse options using boost::program_options
  options_description desc("Allowed options:");
  // clang-format off
  desc.add_options()
    ("help,h", "Produce this help message")
    ("input-file,i", value<string>(), "Input file")
    ("stamp,s", value<string>(), "Image to embed")
    ("output-file,o", value<string>(), "Output file")
    ("side", value<int>(), "Side of the document: 0 center (default), 1 left, 2 right")
    ("rotate", value<int>(), "Assume page is rotated by 0/90/180/270 degrees")
    ("qr", value<string>(), "Add QR instead of image using the specified text");
  // clang-format on

  positional_options_description posDesc;
  posDesc.add("input-file", 1);
  posDesc.add("stamp", 1);
  posDesc.add("output-file", 1);
  posDesc.add("side", 1);
  posDesc.add("rotate", 1);
  posDesc.add("qr", 1);

  variables_map vm;
  store(command_line_parser(argc, argv).options(desc).positional(posDesc).run(),
        vm);
  notify(vm);

  if (vm.count("input-file")) {
    cliOption["inputPDF"] =
        QVariant(QString::fromStdString(vm["input-file"].as<string>()));
  }

  if (vm.count("output-file")) {
    cliOption["outputPDF"] =
        QVariant(QString::fromStdString(vm["output-file"].as<string>()));
  }

  if (vm.count("stamp")) {
    cliOption["imageFile"] =
        QVariant(QString::fromStdString(vm["stamp"].as<string>()));
  }

  if (vm.count("side")) {
    cliOption["side"] = QVariant(vm["side"].as<int>());
  }

  if (vm.count("rotate")) {
    assumeRotate = vm["rotate"].as<int>();
    if ((assumeRotate != 0) && (assumeRotate != 90) && (assumeRotate != 180) &&
        (assumeRotate != 270)) {
      cout << "Valid rotate values are 0,90,180 or 270 only." << endl;
      exit(2);
    }

    cliOption["rotate"] = assumeRotate;
  }

  if (vm.count("qr")) {
    qrText = vm["qr"].as<string>();

    cliOption["qrText"] = QVariant(QString::fromStdString(qrText));
  }

  if ((cliOption.contains("qrText") && cliOption.contains("imageFile")) ||
      (argc < 4) ||
      (!cliOption.contains("qrText") && !cliOption.contains("imageFile")) ||
      vm.empty() || vm.count("help")) {
    cout << desc << endl;
    exit(1);
  }

  return cliOption;
}