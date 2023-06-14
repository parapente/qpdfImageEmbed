# qpdfImageEmbed
A utility to embed an image or a QR into a pdf file.
## Dependencies
- qpdf (v11 or later)
- boost_program_options
- ImageMagick

## Build
Install the necessary packages according to your linux distribution.

In Debian/Ubuntu:
```
apt install libqpdf-dev libboost-program-options-dev libmagick++-dev cmake gcc
```

Then configure and compile the program:
```
mkdir build
cd build
cmake .. && make
```

## Available options
```
  -h [ --help ]            Produce this help message
  -i [ --input-file ] arg  Input file
  -s [ --stamp ] arg       Image to embed
  -o [ --output-file ] arg Output file
  --side arg (=0)          Side of the document: 0 center (default), 1 left, 2 
                           right
  --rotate arg (=0)        Assume page is rotated by 0/90/180/270 degrees
  --qr arg                 Add QR instead of image using the specified text
  --link                   QR value is a URL. Add clickable link
  --scale arg (=1)         Scale image by a factor eg. 0.5
  --top-margin arg (=10)   Set a margin for the image placement from the top of
                           the page
  --side-margin arg (=15)  Set a margin for the image placement from the sides 
                           of the page
  --debug                  Print extra debug messages

```
