#include "pch.h"

#include <iostream>
#include <crtdbg.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

namespace testing
{
    class MemoryLeakDetector final : public EmptyTestEventListener
    {
#ifdef _DEBUG
    public:
        void OnTestStart(const TestInfo&) override
        {
            _CrtMemCheckpoint(&memState_);
        }

        void OnTestEnd(const TestInfo& test_info) override {
            if (test_info.result()->Passed())
            {
                _CrtMemState stateNow, stateDiff;
                _CrtMemCheckpoint(&stateNow);
                if (_CrtMemDifference(&stateDiff, &memState_, &stateNow))
                {
                    FAIL() << "Memory leak of " << stateDiff.lSizes[1] << " byte(s) detected.";
                }
            }
        }

    private:
        _CrtMemState memState_{};
#endif // _DEBUG
    };
}

GTEST_API_ int main(int argc, char** argv)
{
    cout << "Running main() from gtest_mld_main.cpp" << endl;

    InitGoogleTest(&argc, argv);
    UnitTest::GetInstance()->listeners().Append(new MemoryLeakDetector());

    return RUN_ALL_TESTS();
}