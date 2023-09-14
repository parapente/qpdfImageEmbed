# qpdfImageEmbed

A utility to embed an image or a QR into a pdf file.
## Dependencies

- qpdf (v11 or later)
- boost_program_options
- ImageMagick
- libqrencode

## Build

Install the necessary packages according to your linux distribution.

In Debian/Ubuntu:

```bash
apt install libqpdf-dev libboost-program-options-dev libmagick++-dev libqrencode-dev cmake gcc
```

Then configure and compile the program:

```bash
mkdir build
cd build
cmake .. && make
```

## Install

After the compilation has completed successfully we execute:

```bash
make install
```

## Available options

```
Generic:
  -h [ --help ]                Produce this help message
  -i [ --input-file ] arg      Input file
  -o [ --output-file ] arg     Output file
  --rotate arg (=0)            Assume page is rotated by 0/90/180/270 degrees
  --debug                      Print extra debug messages

Image:
  -s [ --stamp ] arg           Image to embed
  --img-side arg (=0)          Side of the document: 0 center (default), 1 
                               left, 2 right
  --img-scale arg (=1)         Scale image by a factor eg. 0.5
  --img-top-margin arg (=10)   Set a margin for the image placement from the 
                               top of the page
  --img-side-margin arg (=15)  Set a margin for the image placement from the 
                               sides of the page

QR:
  --qr arg                     Add QR instead of image using the specified text
  --link                       QR value is a URL. Add clickable link
  --qr-side arg (=0)           Side of the document: 0 center (default), 1 
                               left, 2 right
  --qr-scale arg (=1)          Scale QR by a factor eg. 0.5
  --qr-top-margin arg (=10)    Set a margin for the QR placement from the top 
                               of the page
  --qr-side-margin arg (=15)   Set a margin for the QR placement from the sides
                               of the page

Text (only in latin alphabet):
  --add-text arg               Add extra text to the first page. It can take 
                               multiple strings of the form 
                               '[x,y:][size:][style:]text' where x,y are the 
                               coordinates where the text will appear, size is 
                               a float and gives the font size of the text and 
                               style can be 'i', 'b', 'bi', 'ib' for italic, 
                               bold and bold+italic

```
