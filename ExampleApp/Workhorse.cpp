
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "RouteTrace.h"
#include "Workhorse.h"

Workhorse* global_workhorse = nullptr;


//====================
#pragma mark ctor/dtor

Workhorse::Workhorse(const char*  inputData,
                     const size_t inputSize)
{
    rtTrace();

    assert(inputData != nullptr);
    assert(inputSize > 0);
    data = inputData;
    dataSize = inputSize;
}

Workhorse::~Workhorse()
{
    rtTrace();

}

//--------------------
#pragma mark -

Result Workhorse::doSomeStuff()
{
    rtTrace();

    assert(data != nullptr);
    assert(dataSize > 0);
    if (   data == nullptr
        || dataSize == 0)
    {
        return Result_Failure;
    }

    Result phaseResult = doThisFirst();
    if (phaseResult != Result_Success)
    {
        return phaseResult;
    }

    phaseResult = thenThat();
    if (phaseResult != Result_Success)
    {
        return phaseResult;
    }

    return andLastlyThis();
}

Result Workhorse::doThisFirst()
{
    rtTrace();

    return Result_Success;
}

Result Workhorse::thenThat()
{
    rtTrace();

    return Result_Success;
}

Result Workhorse::andLastlyThis()
{
    rtTrace();

    return Result_Success;
}
