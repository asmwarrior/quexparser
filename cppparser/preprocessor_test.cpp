#include <iostream>
/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#include <fstream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include<iomanip>

#include "preprocessor.h"

int main()
{
    Preprocessor cpp;
    cpp.LoadFile("test_function.cpp");
    cpp.RunTest();
    std::cout<<"------------DUMP MACRO----------------------------\n";
    cpp.DumpMacroTable();
    std::cout<<"------------DUMP Token List----------------------- \n";
    cpp.DumpTokenList();
    //cpp.RunTestPerformance();
    return 0;
}
