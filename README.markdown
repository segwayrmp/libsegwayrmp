# libsegwayrmp

## Documentation

http://wjwwood.github.com/libsegwayrmp/0.1-rc2/

## Dependencies

* CMake, for the build system: http://www.cmake.org/
* Boost, for threading: http://www.boost.org/
* FTDI D2XX (optional), for ftd2xx based usb support: http://www.ftdichip.com/Drivers/D2XX.htm
* Serial (optional), for serial based communication: https://github.com/wjwwood/serial

## Installation

Get the source:

    git clone git://github.com/wjwwood/libsegwayrmp.git
    cd libsegwayrmp

Compile the code:

    make

Or run cmake youself:

    mkdir build && cd build
    cmake ..
    make

Install the code (UNIX):

    make
    sudo make install

Uninstall the code (UNIX):

    make
    sudo make uninstall

Run the test (Requires GTest):

    make test

Build the documentation:

    make doc

## License

The BSD License

Copyright (c) 2011 William Woodall

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
