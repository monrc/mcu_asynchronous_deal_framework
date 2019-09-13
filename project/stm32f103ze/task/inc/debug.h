#ifndef DEBUG_H
#define DEBUG_H

#include "task_config.h"


#ifndef TASK_PLATFORM_DIAG
	#define TASK_PLATFORM_DIAG(x) do {printf x;} while(0)
	#include <stdio.h>
	#include <stdlib.h>
#endif


#ifndef TASK_PLATFORM_ASSERT
#define TASK_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\r\n", \
			x, __LINE__, __FILE__);while(1);} while(0)
#endif

#ifndef TASK_NOASSERT
#define TASK_ASSERT(message, assertion) do { if (!(assertion)) { \
			TASK_PLATFORM_ASSERT(message); }} while(0)
#else  /* TASK_NOASSERT */
#define TASK_ASSERT(message, assertion)
#endif /* TASK_NOASSERT */




#ifndef TASK_PLATFORM_ERROR
#define TASK_PLATFORM_ERROR(message) do {printf("Error \"%s\" failed at line %d in %s\r\n", \
			message, __LINE__, __FILE__);} while(0)
#elif defined TASK_DEBUG
#define TASK_PLATFORM_ERROR(message) TASK_PLATFORM_DIAG((message))
#else
#define TASK_PLATFORM_ERROR(message)
#endif

/* if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef TASK_NOERROR
#define TASK_ERROR(message, expression, handler) do { if (!(expression)) { \
			TASK_PLATFORM_ERROR(message); handler;}} while(0)
#else
#define TASK_ERROR(message, expression, handler) do{ if (!(expression)){handler;}}while(0)
#endif /* TASK_ERROR */


#ifdef TASK_DEBUG
#ifndef TASK_PLATFORM_DIAG
	#error "If you want to use TASK_DEBUG, TASK_PLATFORM_DIAG(message) needs to be defined in your arch/cc.h"
#endif
#define TASK_DEBUGF(debug, message) do { \
		if ( \
				((debug) & TASK_DBG_ON) && \
				((debug) & TASK_DBG_TYPES_ON) && \
				((s16_t)((debug) & TASK_DBG_MASK_LEVEL) >= TASK_DBG_MIN_LEVEL)) { \
			TASK_PLATFORM_DIAG(message); \
			if ((debug) & TASK_DBG_HALT) { \
				while(1); \
			} \
		} \
	} while(0)

#else  /* TASK_DEBUG */
#define TASK_DEBUGF(debug, message)
#endif /* TASK_DEBUG */

#endif