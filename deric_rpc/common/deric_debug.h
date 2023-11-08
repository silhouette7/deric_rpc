#ifndef _DERIC_DEBUG_H_
#define _DERIC_DEBUG_H

#ifdef DEBUG_ENABLE
#ifdef DEBUG_INTO_FILE
#else
#define DEBUG_ERROR(format,...) printf("ERROR:%s:%s():%d: " format "\n",__FILE__,__func__, __LINE__, ##__VA_ARGS__)
#define DEBUG_INFO(format,...) printf("INFO:%s:%s():%d: " format "\n",__FILE__,__func__, __LINE__, ##__VA_ARGS__)
#endif
#else
#define DEBUG_ERROR
#define DEBUG_INFO
#endif

#endif