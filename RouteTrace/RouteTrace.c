
/* RouteTrace v1.0 */

#include "RouteTrace.h"     /* included first because of SYSLOG_EXISTS */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#if(SYSLOG_EXISTS)
#include <sys/syslog.h>
#endif

/* ------------------------------ */

static const char* const traceMessagePrefix     = "TRACE:";
static const char* const traceListHeader        = "TRACE LIST:";
static const char* const traceListBorder        = "--------------------";
static const char* const traceFailWrongOrder    = "RouteTrace test failed: Route '%s' is not in the correct order.";
static const char* const traceFailNotFound      = "RouteTrace test failed: Route '%s' was not found.";

/* ------------------------------ */

static unsigned int numTraces = 0;
static char** traces = NULL;

static unsigned int numSubjects = 0;
static RTSubject* subjects[MAX_SUBJECTS];

#define TRACE_NOT_FOUND     UINT_MAX

/* ============================== */

static void displayMessage(const char* const message,
                           ...)
{
    assert(message);

    {
        va_list args;
        bool shouldAddLineFeed = yes;

        const size_t messageLength = strlen(message);
        assert(messageLength > 0);

        if (message[messageLength - 1] == '\n')
        {
            shouldAddLineFeed = no;
        }

        /* debug console */
        va_start(args, message);
        vprintf(message, args);
        if (shouldAddLineFeed)
        {
            printf("\n");
        }
        va_end(args);

        #if(SYSLOG_EXISTS)
        /* log file */
        va_start(args, message);
        vsyslog(LOG_NOTICE, message, args);
        va_end(args);
        #endif
    }
}

/* ============================== */

static bool routesAreInCorrectOrderForSubject(const RTSubject* const subject,
                                              unsigned int* const    traceIndices)
{
    assert(subject);
    assert(traceIndices);

    {
        unsigned int r;
        for (r = 1; r < subject->numRoutes; ++r)
        {
            const unsigned int previousIndex = traceIndices[r - 1];
            const unsigned int currentIndex = traceIndices[r];
            unsigned int       nextIndex = currentIndex;

            if (r < subject->numRoutes - 1)
            {
                nextIndex = traceIndices[r + 1];
            }

            if (   currentIndex < previousIndex
                || currentIndex > nextIndex)
            {
                RTRoute* route = subject->routes[r];
                assert(route);
                if (route)
                {
                    assert(route->routine);
                    displayMessage(traceFailWrongOrder, route->routine);
                }

                return no;
            }
        }

        return yes;
    }
}

static bool routineMatchesTrace(const char* const routine,
                                const char* const trace)
{
    if (   routine == NULL
        || strlen(routine) == 0
        || trace == NULL
        || strlen(trace) == 0)
    {
        return no;
    }

    if (strstr(trace, routine))
    {
        return yes;
    }

    return no;
}

static unsigned int traceIndexForRoutine(const char* const routineName)
{
    if (   routineName == NULL
        || strlen(routineName) == 0)
    {
        return TRACE_NOT_FOUND;
    }

    {
        unsigned int t;
        for (t = 0; t < numTraces; ++t)
        {
            char* trace = traces[t];
            assert(trace != NULL);
            if (trace && routineMatchesTrace(routineName, trace))
            {
                return t;
            }
        }

        return TRACE_NOT_FOUND;
    }
}

static bool isRoutePresentForSubject(const RTSubject* const subject,
                                     unsigned int* const    traceIndices)
{
    assert(subject);
    assert(traceIndices);

    {
        unsigned int r;
        assert(subject->numRoutes < MAX_ROUTES);
        for (r = 0; r < subject->numRoutes; r++)
        {
            RTRoute* route = subject->routes[r];
            assert(route);
            if (route)
            {
                assert(route->routine);
                traceIndices[r] = traceIndexForRoutine(route->routine);
                if (traceIndices[r] == TRACE_NOT_FOUND)
                {
                    displayMessage(traceFailNotFound, route->routine);
                    return no;
                }
            }
        }

        return yes;
    }
}

/* ------------------------------ */

bool rtTracePassedForSubject(const RTSubject* const subject)
{
    if (   subject == NULL
        || numTraces == 0)
    {
        return no;
    }

    {
        const size_t        indexByteSize = sizeof(unsigned int) * numTraces;
        unsigned int* const traceIndices = calloc(1, indexByteSize);
        if (traceIndices == NULL)
        {
            return no;
        }

        if (   isRoutePresentForSubject(subject, traceIndices) == no
            || routesAreInCorrectOrderForSubject(subject, traceIndices) == no)
        {
            free(traceIndices);
            return no;
        }

        free(traceIndices);
        return yes;
    }
}

/* ============================== */

static void destroyRoute(RTRoute* route)
{
    assert(route);

    if (route->routine)
    {
        free(route->routine);
    }

    free(route);
}

/* ------------------------------ */

void rtAddRouteToSubject(RTSubject* const  subject,
                         const char* const routineName)
{
    if (   subject == NULL
        || routineName == NULL
        || strlen(routineName) == 0
        || subject->numRoutes >= MAX_ROUTES)
    {
        return;
    }

    {
        const size_t routeSize = sizeof(RTRoute);
        RTRoute*     route = calloc(1, routeSize);
        if (route == NULL)
        {
            return;
        }

        route->routine = strdup(routineName);
        if (route->routine == NULL)
        {
            destroyRoute(route);
            return;
        }

        subject->routes[subject->numRoutes] = route;
        subject->numRoutes++;
    }
}

/* ============================== */

static void addSubjectToList(RTSubject* subject)
{
    assert(subject);

    assert(numSubjects < MAX_SUBJECTS);
    if (numSubjects < MAX_SUBJECTS)
    {
        subjects[numSubjects] = subject;
        numSubjects++;
    }
}

static void destroyRoutesForSubject(const RTSubject* const subject)
{
    assert(subject);

    {
        unsigned int r;
        for (r = 0; r < subject->numRoutes; ++r)
        {
            RTRoute* route = subject->routes[r];
            assert(route != NULL);
            if (route)
            {
                destroyRoute(route);
            }
        }
    }
}

static void destroySubject(RTSubject* subject)
{
    if (subject == NULL)
    {
        return;
    }

    if (subject->module)
    {
        free(subject->module);
    }
    if (subject->routine)
    {
        free(subject->routine);
    }

    destroyRoutesForSubject(subject);

    free(subject);
}

/* ------------------------------ */

RTSubject* rtAddSubject(const char* const moduleName,
                        const char* const routineName)
{
    if (   moduleName == NULL
        || strlen(moduleName) == 0
        || routineName == NULL
        || strlen(routineName) == 0
        || numSubjects >= MAX_SUBJECTS)
    {
        return NULL;
    }

    {
        const size_t subjectSize = sizeof(RTSubject);
        RTSubject*   subject = calloc(1, subjectSize);
        if (subject == NULL)
        {
            return NULL;
        }

        subject->module = strdup(moduleName);
        subject->routine = strdup(routineName);

        if (   subject->module == NULL
            || subject->routine == NULL)
        {
            destroySubject(subject);
            return NULL;
        }

        addSubjectToList(subject);

        return subject;
    }
}

/* ============================== */

static void displayAllTraces()
{
    unsigned int t;
    for (t = 0; t < numTraces; ++t)
    {
        char* trace = traces[t];
        assert(trace);
        if (trace)
        {
            displayMessage(trace);
        }
    }
}

/* ------------------------------ */

void rtDisplayTraces()
{
    displayMessage(traceListBorder);
    displayMessage(traceListHeader);
    displayAllTraces();
    displayMessage(traceListBorder);
}

/* ============================== */

static void addTraceToTable(const char* const message)
{
    assert(message != NULL);
    assert(strlen(message) > 0);

    {
        char* messageCopy = strdup(message);
        if (messageCopy)
        {
            char** newTraces = realloc(traces, sizeof(char*) * (numTraces + 1));
            if (newTraces)
            {
                traces = newTraces;
                traces[numTraces] = messageCopy;
                numTraces++;
                return;
            }

            free(messageCopy);
        }
    }
}

/* ------------------------------ */

void rtAddTrace(const char* const message)
{
    if (   message == NULL
        || strlen(message) == 0)
    {
        return;
    }

    addTraceToTable(message);
    displayMessage("%s %s", traceMessagePrefix, message);
}

/* ============================== */

static void destroySubjects()
{
    unsigned int s;
    for (s = 0; s < numSubjects; ++s)
    {
        RTSubject* subject = subjects[s];
        assert(subject != NULL);
        if (subject)
        {
            destroySubject(subject);
        }
    }
}

static void initializeSubjects()
{
    unsigned int s;
    for (s = 0; s < MAX_SUBJECTS; ++s)
    {
        subjects[s] = NULL;
    }
    numSubjects = 0;
}

static void destroyTraces()
{
    unsigned int t;
    for (t = 0; t < numTraces; ++t)
    {
        char* trace = traces[t];
        assert(trace != NULL);
        if (trace)
        {
            free(trace);
        }

        traces[t] = NULL;
    }
}

static void initializeTraces()
{
    numTraces = 0;
    traces = NULL;
}

/* ============================== */

void rtTearDown()
{
    destroyTraces();
    destroySubjects();
}

void rtSetUp()
{
    initializeTraces();
    initializeSubjects();
}
