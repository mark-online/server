#include "CommunityServerTestPCH.h"
#include <sne/base/utility/LogManager.h>

int main(int argc, char* argv[])
{
    sne::base::LogManager logManager;
    logManager.silent();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
