// Copyright (c) 2018-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/loader.h>

#ifdef WIN32
__declspec(dllexport) int main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    return GuiMain(argc, argv); 
}
