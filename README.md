## Overview

RouteTrace is a testing- and debugging tool that enforces an expected code flow. It does this by comparing routes through your code with the expectations you establish for it.

Traces are added to your routines that act as a sort-of 'code flow map'. Another way to think of it is that traces are an "I am here" marker throughout your code. By checking the order of these markers, you can ensure that the routines are called in the correct sequence.

The intent is for traces to stay included in your release builds so they can be added to the system log. If a crash occurs, for example, the traces are included to help you determine the cause. In many cases the traces are more useful than a call stack.

### Introducing RDD

Route Driven Development (RDD) is a new, additional method by which you can test your code. Not quite as low-level as a unit test, but not as high level as a behavior-, acceptance-, or even a feature test. This kind of testing is somewhere in-between, though closer to unit testing. Essentially it's a way to fortify your tracer bullets.

To paraphrase [Wikipedia](https://en.wikipedia.org/wiki/Scrum_%28software_development%29) and [Andy Hunt/Dave Thomas](https://www.amazon.com/Pragmatic-Programmer-Journeyman-Master/dp/020161622X), tracer bullets are a quick way to get from a requirement to production code, usually resulting in end-to-end functionality. That is, the a requirement is implemented with stubs, but every part of the requirement is represented with shippable code.

You use RDD to create the skeleton of your program, knowing that the sequence is always what you expect it to be (provided you continue to run the tests, of course).

The method by which you practice RDD is very similar to how you would practice [Test Driven Development](https://en.wikipedia.org/wiki/Test-driven_development) (TDD), just at a different level. If you think of testing and the xDD's in general, they are like layers of an onion. The outer layers are more high-level ([BDD](https://en.wikipedia.org/wiki/Behavior-driven_development), [ATDD](https://en.wikipedia.org/wiki/Acceptance_test-driven_development), [FDD](https://en.wikipedia.org/wiki/Feature-driven_development); i.e. macro) testing while the inner layers are very focused, low-level (i.e. micro) testing.

The sequence of TDD is:

1. red
2. green
3. refactor

That is, you create a test first and see it fail (the bar goes red). Then you do the smallest amount of work necessary to get the test to pass ("What is the simplest thing that could possibly work?"; the bar turns green.) Finally, you refactor the code to remove duplication, magic numbers, and anything else that smells. Afterwords, you run the tests again to make sure they all still pass.

This sequence is exactly the same with RDD except that instead of testing specific functionality, you're testing that the higher-order functions exist _and are called in the correct order_.

Once you have your tracer bullet (or indeed, the skeleton of your program or algorithm) complete, you run the RDD tests alongside any other testing you have in place. RDD is just another way to ensure that any future refactoring allows code flow to continue to be what you expect.

## Installation

Installation is very easy: simply copy the two RouteTrace files into your project. I wrote it to C99 to try to be as usable by as many developers as possible.

It works perfectly with Xcode for use in C++ and Objective-C applications and fits seamlessly in Xcode's XCTesting mechanism. I believe it is generic enough to fit into most projects/testing suites (but please create an issue if that's not the case).

If there is great enough demand, I will create a Cocoapod for it.

## Usage

### rtSetUp / rtTearDown

Using RouteTrace is quite straightforward. Early in your program, you call `rtSetUp()` to initialize the tool and prepare it for use. When your program is ready to terminate, you would call `rtTearDown()` to free up any resources RouteTrace might be consuming. This pair of functions can also be used when unit testing; they correspond directly to `setUp` and `tearDown` in Xcode's XCTest mechanism.

### rtAddTrace / rtTrace

`rtAddTrace(const char* const message);` is used to add a message to the trace log. This message can be anything you wish, but a helper macro `rtTrace()` exists that makes it easy for any function to add a trace message that is the method signature.

For example:

```cpp
Result loadFile(const char* path)
{
    rtTrace();
    //...
}
```

Would show
```
TRACE: Result loadFile(const char *)
```
in the log.

When used at the top of a method, you can use `rtTrace()` to add breadcrumbs to the log as a sort of 'code flow map'. This is very handy for debugging crashes, for example, because you can follow the exact path the code took (more useful than a call stack in a number of cases).

### rtDisplayTraces

If you wanted to see the current list of traces, you can call `rtDisplayTraces()`, which would display something like this:

```
--------------------
TRACE LIST:
int main(int, const char **)
Result init(int, const char **)
void initState()
Result parseCommandLine(int, const char **)
Result doSomeProperWork()
Result loadFile(const char *)
size_t fileSize(const char *const)
Result launchWorkhorse(const char *const, const size_t)
Workhorse::Workhorse(const char *, const size_t)
Result Workhorse::doSomeStuff()
Result Workhorse::doThisFirst()
Result Workhorse::thenThat()
Result Workhorse::andLastlyThis()
--------------------
```

## Unit Testing Features

As mentioned earlier, the main strength of RouteTrace is to enforce an expected code flow. You do this by writing unit-like tests to ensure that a specific flow exists in your code. If a route is missing or in the wrong order, the test will fail.

There are three functions available for use with testing:
```
extern RTSubject* rtAddSubject(const char* const moduleName,
                               const char* const routineName);

extern void rtAddRouteToSubject(RTSubject* const  subject,
                                const char* const routineName);

extern bool rtTracePassedForSubject(const RTSubject* const subject);
```

### rtAddSubject

`rtAddSubject(moduleName, routineName)` is used to create a Subject that will be scrutinized. `moduleName` and `routineName` are just identifiers for the code module (file/class/etc.) and a routine in that module. They are just strings so they can technically be anything you want but keeping to the moduleName/routineName system is a good idea because they indicate where the code maintainer should start looking if a test fails.

### rtAddRouteToSubject

`rtAddRouteToSubject(subject, routineName)` is the way to tell RouteTrace what the expectations are for the test. Up to 64 routes can be added to a subject and they are added in the order you expect the flow to take.

### rtTracePassedForSubject

`rtTracePassedForSubject(subject)` is called when it's time to actually run the test.

To illustrate, here is a test found in the example project:

```objc
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
```

Here, I am testing the `doSomeStuff` routine found in the `Workhorse` class. I expect `doThisFirst` to be called first, followed by `thenThat`, and finally `andLastlyThis`. I invoke the test by calling `rtTracePassedForSubject()` via XCTest's `XCTAssert()` call.

In order for this test to pass, I need to add breadcrumbs in my code, at least in those three routines.

```cpp
Result Workhorse::doThisFirst()
{
    rtTrace();
    //...
    return Result_Success;
}

Result Workhorse::thenThat()
{
    rtTrace();
    //...
    return Result_Success;
}

Result Workhorse::andLastlyThis()
{
    rtTrace();
    //...
    return Result_Success;
}
```

Let's say after some refactoring work, the order has changed. `thenThat()` is now being called first. The test would fail and present output something like this:

```
TRACE: Workhorse::Workhorse(const char *, const size_t)
TRACE: Result Workhorse::doSomeStuff()
TRACE: Result Workhorse::thenThat()
TRACE: Result Workhorse::doThisFirst()
TRACE: Result Workhorse::andLastlyThis()
RouteTrace test failed: Route 'thenThat' is not in the correct order.
```

Another example is if I remove a call altogether (let's say I got a bit overzealous with optimizations and determined that `thenThat()` was no longer needed. I would see the test fail with the following output:

```
TRACE: Workhorse::Workhorse(const char *, const size_t)
TRACE: Result Workhorse::doSomeStuff()
TRACE: Result Workhorse::doThisFirst()
TRACE: Result Workhorse::andLastlyThis()
RouteTrace test failed: Route 'thenThat' was not found.
```

I would either have to undo some of my refactoring, or update my test.

## RouteTrace.h

There are a couple of notes to make about RouteTrace.h.

First, there is a define at the top called `STDBOOL_EXISTS`. If your compiler has `<stdbool.h>`, leave `STDBOOL_EXISTS` set to 1. If `<stdbool.h>` doesn't exist, change it to 0 so the booleans can get defined.

There is another define called `SYSLOG_EXISTS`. If your compiler implements `syslog()`, leave `SYSLOG_EXISTS` set to 1. Otherwise change it to 0. When `SYSLOG_EXISTS` is 1, RouteTrace will add each trace message to the system log as well as displaying it in the console. This is a handy feature when analyzing crash logs (for example) because you're able see the traces which lead up to the crash.

Also, I have capped the number of routes per subject to 64 (via `MAX_SUBJECTS`). This seems like a reasonable limitation to me because a `Subject` is only used for testing. If you feel constricted by this limitation, feel free to increase `MAX_SUBJECTS`.

If there is a demand for it, this can be turned into a dynamic system but I opted for simplicity over expandability for the initial release.

Finally, there is `AUTO_RT_FUNC`. This is used by `rtTrace()` so you wouldn't use this directly. I've tried to make it as usable as possible by checking for the existence of things and hopefully it will use the appropriate version (`__PRETTY_FUNCTION__`, `__func__`, etc.) for you. If you run into problems with your compiler, please create a pull request that has it fixed and I'll incorporate it into the next release.

## Demo Project

An example Xcode (Mac command line) project is provided. It's just a skeleton program that doesn't do anything useful; it's a playground of sorts where you can modify what's there and see how the tracing and testing mechanism works. I have left it in a state where all tests pass so you can see how to set it up properly, but feel free to add your own tests and see them fail/pass.
 
## Author

[Bob Koon](http://www.bobkoon.com)

## License

Copyright © 2016 Binary Hammer LLC

RouteTrace is available under the MIT license. See the LICENSE file for more info.
