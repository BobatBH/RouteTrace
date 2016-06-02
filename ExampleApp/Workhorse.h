#pragma once

#include "Result.h"


class Workhorse
{
    public:
        // ctor/dtor
        Workhorse(const char*  inputData,
                  const size_t inputSize);
        ~Workhorse();

        // features
        Result doSomeStuff();

    private:
        Result doThisFirst();
        Result thenThat();
        Result andLastlyThis();

    private:
        const char* data;     // convenience variables
        size_t dataSize;
};

extern Workhorse* global_workhorse;
