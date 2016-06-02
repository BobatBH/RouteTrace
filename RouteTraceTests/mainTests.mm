
#import <XCTest/XCTest.h>
#import "RouteTrace.h"
#import "main.h"

#define sizeofstaticarray(array)   (sizeof(array) / sizeof((array)[0]))

@interface mainTests : XCTestCase

@end


#pragma mark -

@implementation mainTests

- (void)setUp
{
    [super setUp];
    rtSetUp();
}

- (void)tearDown
{
    rtTearDown();
    [super tearDown];
}

#pragma mark -

- (void)testMainInitStateCalledBeforeParseCommandLine
{
    // set up the expectations
    RTSubject* subject = rtAddSubject("main", "init");
    if (subject)
    {
        rtAddRouteToSubject(subject, "initState");
        rtAddRouteToSubject(subject, "parseCommandLine");

        // perform the test
        const char* argv[] = { "ExampleApp" };
        init(sizeofstaticarray(argv), &argv[0]);
        XCTAssert(rtTracePassedForSubject(subject));
    }
}

@end
