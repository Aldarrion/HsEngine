#include "UnitTests.h"

#include <stdio.h>
#include <stdlib.h>


int main()
{
    hsTest::g_TestCollection.RunTests();

    #if HS_WINDOWS
        //system("pause");
    #endif
}
