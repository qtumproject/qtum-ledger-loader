What is Qtum Ledger Loader?
-------------

Qtum Ledger Loader is a tool to communicate with ledger Nano S and manage Qtum ledger applications life cycle.

# Building Qtum Ledger Loader

### Validate and Reproduce Binaries

Qtum uses a tool called Gitian to make reproducible builds that can be verified by anyone. Instructions on setting up a Gitian VM and building Qtum are provided in [Gitan Building](https://github.com/qtumproject/qtum/blob/master/doc/gitian-building.md)
For Qtum Ledger Loader is used `contrib/gitian-build-loader.py` to build it in the Qtum gitian environment.

### Build on Ubuntu

This is a quick start script for compiling Qtum Ledger Loader on Ubuntu


    sudo apt-get install build-essential libtool autotools-dev automake pkg-config bsdmainutils git
    sudo apt-get install software-properties-common
    sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler

    git clone https://github.com/qtumproject/qtum-ledger-loader.git
    cd qtum-ledger-loader

    # Note autogen will prompt to install some more dependencies if needed
    ./autogen.sh
    ./configure 
    make -j2
    
### Build on CentOS

Here is a brief description for compiling Qtum Ledger Loader on CentOS
    
    # Installing Dependencies for Qtum Ledger Loader
    sudo yum install epel-release
    sudo yum install gcc-c++ libtool
    sudo yum install qt5-qttools-devel protobuf-devel
    
    # Building Qtum Ledger Loader
    git clone https://github.com/qtumproject/qtum-ledger-loader.git
    cd qtum-ledger-loader
    ./autogen.sh
    ./configure
    make -j4

### Build on Mac OS

The commands in this guide should be executed in a Terminal application.
The built-in one is located in `/Applications/Utilities/Terminal.app`.

#### Preparation

Install the Mac OS command line tools:

`xcode-select --install`

When the popup appears, click `Install`.

Then install [Homebrew](https://brew.sh).

#### Dependencies

    brew install cmake automake libtool pkg-config protobuf qt5 imagemagick librsvg

#### Build Qtum Ledger Loader

1. Clone the qtum ledger loader source code and cd into `qtum-ledger-loader`

        git clone https://github.com/qtumproject/qtum-ledger-loader.git
        cd qtum-ledger-loader

2.  Build qtum-ledger-loader:

    Configure and build the GUI.

        ./autogen.sh
        ./configure
        make


### Run

Then you can run the Qt GUI using `src/qt/qtum_loader-qt`

License
-------

Qtum Ledger Loader is GPLv3 licensed.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/qtumproject/qtum-ledger-loader/tags) are created
regularly to indicate new official, stable release versions of Qtum Ledger Loader.

The contribution workflow is described in [CONTRIBUTING.md](https://github.com/qtumproject/qtum/blob/master/CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](https://github.com/qtumproject/qtum/blob/master/doc/developer-notes.md).

