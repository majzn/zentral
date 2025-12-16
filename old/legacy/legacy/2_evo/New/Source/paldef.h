#ifndef PAL_DEFINES_H
#define PAL_DEFINES_H

/********************************************/
/* @define                                  */
/* PalLib Win32 Defines                     */
/********************************************/

#undef TRUE
#define TRUE 1

#undef FALSE
#define FALSE 0

typedef void* palHandle;
#define sflag int

#define SLOGERR(x) if(!x) { printf("[palerr] ERROR at: %s, line: %d\n", __FILE__, __LINE__ ); }
#define SLOGINF(x) printf("[palinf] INFO at: %s, line: %d\n", __FILE__, __LINE__ )

#define VERBOSE_PREFIX

#ifdef VERBOSE_PREFIX
#define SLOGPREFIX printf("[%s][%d] > ", __FILE__, __LINE__)
#else
#define SLOGPREFIX while(0)
#endif

#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
#define ENDL printf("\n")
#define SPRINT(x) printf(x)
#define SLOG(x) SLOGPREFIX; printf(x)
#define SLOG1(x, a1) SLOGPREFIX; printf(x, a1)
#define SLOG2(x, a1, a2) SLOGPREFIX; printf(x, a1, a2)
#define SLOG3(x, a1, a2, a3) SLOGPREFIX; printf(x, a1, a2, a3)
#define SLOG4(x, a1, a2, a3, a4) SLOGPREFIX; printf(x, a1, a2, a3, a4)
#define SLOG5(x, a1, a2, a3, a4, a5) SLOGPREFIX; printf(x, a1, a2, a3, a4, a5)
#define SLOG6(x, a1, a2, a3, a4, a5, a6) SLOGPREFIX; printf(x, a1, a2, a3, a4, a5, a6)
#define SLOG7(x, a1, a2, a3, a4, a5, a6, a7) SLOGPREFIX; printf(x, a1, a2, a3, a4, a5, a6, a7)
#define SLOG8(x, a1, a2, a3, a4, a5, a6, a7, a8) SLOGPREFIX; printf(x, a1, a2, a3, a4, a5, a6, a7, a8)
#define SLOG9(x, a1, a2, a3, a4, a5, a6, a7, a8, a9) SLOGPREFIX; printf(x, a1, a2, a3, a4, a5, a6, a7, a8, a9)
#define SLOG10(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) SLOGPREFIX; printf(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
#else
#define ENDL while(0) {}
#define SPRINT(x) while(0) {}
#define SLOG(x) while(0) {}
#define SLOG1(x, a1) while(0) {}
#define SLOG2(x, a1, a2) while(0) {}
#define SLOG3(x, a1, a2, a3) while(0) {}
#define SLOG4(x, a1, a2, a3, a4) while(0) {}
#define SLOG5(x, a1, a2, a3, a4, a5) while(0) {}
#define SLOG6(x, a1, a2, a3, a4, a5, a6) while(0) {}
#define SLOG7(x, a1, a2, a3, a4, a5, a6, a7) while(0) {}
#define SLOG8(x, a1, a2, a3, a4, a5, a6, a7, a8) while(0) {}
#define SLOG9(x, a1, a2, a3, a4, a5, a6, a7, a8, a9) while(0) {}
#define SLOG10(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) while(0) {}
#endif

#define PCHECK(x) printf("%s\n", (x)?"done":"error")

#define ASSERT(x) if(!(x)) { \
printf("ABORTING PROGRAM - FATAL ERROR at: %s, line: %d, time: %s [%s]\n", __FILE__, __LINE__, __TIME__, #x); \
exit(-1); \
}

#ifndef NOMINMAX
 #define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
 #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif

#undef UNICODE

#endif
