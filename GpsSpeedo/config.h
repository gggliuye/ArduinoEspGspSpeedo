



// #define DEBUG_MODE

#if defined(DEBUG_MODE)
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...) ((void)0) // do nothing
#endif
