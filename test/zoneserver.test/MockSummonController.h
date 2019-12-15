#pragma once

#include "MockNpcController.h"
#include <sne/test/CallCounter.h>

using namespace gideon::zoneserver;

/**
 * @class MockSummonController
 *
 * 테스트 용 mock SummonController
 */
class MockSummonController :
    public MockNpcController
{
public:
    MockSummonController() :
        lastErrorCode_(ecWhatDidYouTest) {}

public:
    ErrorCode lastErrorCode_;
};
