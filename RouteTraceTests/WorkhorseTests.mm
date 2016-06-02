
#import <XCTest/XCTest.h>
#import "RouteTrace.h"
#import "Workhorse.h"

@interface WorkhorseTests : XCTestCase

@end


#pragma mark -

@implementation WorkhorseTests

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

- (void)testWorkhorseSteps
{
    // set up the expectations
    RTSubject* subject = rtAddSubject("Workhorse", "doSomeStuff");
    if (subject)
    {
        rtAddRouteToSubject(subject, "doThisFirst");
        rtAddRouteToSubject(subject, "thenThat");
        rtAddRouteToSubject(subject, "andLastlyThis");

        // perform the test
        const char* testData = "Gort, Klaatu Barada Nikto";

        Workhorse* workhorse = new Workhorse(testData, strlen(testData));
        workhorse->doSomeStuff();

        XCTAssert(rtTracePassedForSubject(subject));
    }
}

@end
