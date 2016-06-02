
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <new>
#include "RouteTrace.h"
#include "main.h"
#include "Workhorse.h"

//--------------------
#pragma mark -

static const char* inputFilePath;
static const char* inputFileContents;
static size_t inputFileSize;

static Result launchWorkhorse(const char* const contents,
                              const size_t      contentSize)
{
    rtTrace();

    assert(contents != nullptr);
    assert(contentSize > 0);
    if (   contents == nullptr
        || contentSize == 0)
    {
        return Result_Failure;
    }

    global_workhorse = new (std::nothrow) Workhorse(contents, contentSize);
    if (global_workhorse)
    {
        return global_workhorse->doSomeStuff();
    }

    return Result_Failure;
}

static size_t fileSize(const char* const path)
{
    rtTrace();

    assert(path != nullptr);
    assert(strlen(path) > 0);
    if (   path == nullptr
        || strlen(path) == 0)
    {
        return 0;
    }

    struct stat stat_data;
    const int status = stat(path, &stat_data);
    if (status == 0)
    {
        return (size_t)stat_data.st_size;
    }

    return 0;
}

static Result loadFile(const char* path)
{
    rtTrace();

    assert(path != nullptr);
    assert(strlen(path) > 0);
    if (   path == nullptr
        || strlen(path) == 0)
    {
        return Result_Failure;
    }

    const size_t size = fileSize(path);
    if (size == 0)
    {
        (void)printf("size for '%s' is 0; probably file not found\n", path);
        return Result_NoAction;
    }

    void* contents = malloc(size);
    if (contents == nullptr)
    {
        return Result_Failure;
    }

    FILE* file = fopen(path, "rb");
    if (file == nullptr)
    {
        (void)printf("Could not open '%s'\n", path);
        free(contents);
        return Result_Failure;
    }

    size_t result = fread(contents, 1, size, file);
    if (result != size)
    {
        (void)printf("Could not read '%s'\n", path);
        fclose(file);
        free(contents);
        return Result_Failure;
    }

    fclose(file);

    inputFileContents = (const char*)contents;
    inputFileSize = size;

    return Result_Success;
}

static Result doSomeProperWork()
{
    rtTrace();

    if (loadFile(inputFilePath) == Result_Success)
    {
        return launchWorkhorse(inputFileContents, inputFileSize);
    }

    return Result_Failure;
}

//--------------------
#pragma mark -

static void cleanUp()
{
    rtTrace();

    if (global_workhorse)
    {
        delete global_workhorse;
    }

    if (inputFileContents)
    {
        free((void*)inputFileContents);
    }
}

//--------------------
#pragma mark -

static void displayUsage()
{
    rtTrace();

    (void)printf("Standard usage message\n");
    (void)printf("Usage: ExampleApp [options] <file>\n\n");
}

static Result parseCommandLine(int         argc,
                               const char* argv[])
{
    rtTrace();

    if (argc == 1)
    {
        displayUsage();
        return Result_NoAction;
    }

    if (argc != 2)  // contrived just for the example
    {
        (void)printf("No file was specified\n");
        return Result_Failure;
    }

    inputFilePath = argv[1];
    (void)printf("inputFilePath: '%s'\n", inputFilePath);

    return Result_Success;
}

static void initState()
{
    rtTrace();

    inputFilePath = nullptr;
    inputFileContents = nullptr;
    inputFileSize = 0;
}

Result init(int         argc,
            const char* argv[])
{
    rtTrace();
    initState();
    return parseCommandLine(argc, argv);
}

//====================
#pragma mark -

int main(int         argc,
         const char* argv[])
{
    rtTrace();

    Result result = init(argc, argv);
    if (result == Result_NoAction)
    {
        result = Result_Success;
    }
    else
    if (result == Result_Success)
    {
        result = doSomeProperWork();
    }
rtDisplayTraces();

    cleanUp();
    return result;
}
