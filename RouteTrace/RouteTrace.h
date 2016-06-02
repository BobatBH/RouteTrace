
/* RouteTrace v1.0 */

/* set STDBOOL_EXISTS to 1 if the compiler can find <stdbool.h> */
#define STDBOOL_EXISTS  1

#if(STDBOOL_EXISTS)
    #include <stdbool.h>
#else
    #ifndef bool
        #define bool char
    #endif
    #ifndef true
        #define true 1
    #endif
    #ifndef false
        #define false 0
    #endif
#endif
#ifndef yes
    #define yes true
#endif
#ifndef no
    #define no false
#endif

/* set SYSLOG_EXISTS to 1 if the compiler implements syslog() */
#define SYSLOG_EXISTS   1


typedef struct RTROUTE
{
    char* routine;
} RTRoute;

#define MAX_ROUTES      64

typedef struct RTSUBJECT
{
    char* module;
    char* routine;

    unsigned int numRoutes;
    RTRoute* routes[MAX_ROUTES];
} RTSubject;

#define MAX_SUBJECTS    64


#if (__GNUC__ || __GNUG__)
    #define AUTO_RT_FUNC    __PRETTY_FUNCTION__
#elif defined(__func__)
    #define AUTO_RT_FUNC    __func__
#else
    #define AUTO_RT_FUNC    __FUNCTION__
#endif

#define rtTrace()   rtAddTrace(AUTO_RT_FUNC)


#if defined(__cplusplus)
extern "C"
{
#endif

extern void rtSetUp();
extern void rtTearDown();

extern void rtAddTrace(const char* const message);

extern void rtDisplayTraces();

/* unit testing features */
extern RTSubject* rtAddSubject(const char* const moduleName,
                               const char* const routineName);

extern void rtAddRouteToSubject(RTSubject* const  subject,
                                const char* const routineName);

extern bool rtTracePassedForSubject(const RTSubject* const subject);

#if defined(__cplusplus)
}
#endif
