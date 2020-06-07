#ifndef DEBUG_H
#define DEBUG_H

#include "task_config.h"


#ifndef PLATFORM_DIAG
	#define PLATFORM_DIAG(x) do {printf x;} while(0)
	#include <stdio.h>
	#include <stdlib.h>
#endif


#ifndef PLATFORM_ASSERT
#define PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\r\n", \
			x, __LINE__, __FILE__);while(1);} while(0)
#endif

#ifndef NOASSERT
#define TASK_ASSERT(message, assertion) do { if (!(assertion)) { \
			PLATFORM_ASSERT(message); }} while(0)
#else  /* NOASSERT */
#define TASK_ASSERT(message, assertion)
#endif /* NOASSERT */




#ifndef PLATFORM_ERROR
#define PLATFORM_ERROR(message) do {printf("Error \"%s\" failed at line %d in %s\r\n", \
			message, __LINE__, __FILE__);} while(0)
#elif defined DEBUG
#define PLATFORM_ERROR(message) PLATFORM_DIAG((message))
#else
#define PLATFORM_ERROR(message)
#endif

/* if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef NOERROR
#define ERROR(message, expression, handler) do { if (!(expression)) { \
			PLATFORM_ERROR(message); handler;}} while(0)
#else
#define ERROR(message, expression, handler) do{ if (!(expression)){handler;}}while(0)
#endif /* ERROR */


#ifndef DEBUG
#ifndef PLATFORM_DIAG
	#error "If you want to use DEBUG, PLATFORM_DIAG(message) needs to be defined in your arch/cc.h"
#endif
#define DEBUG(debug, message) do { \
		if ( \
				((debug) & TASK_DBG_ON) && \
				((debug) & TASK_DBG_TYPES_ON) && \
				((s16_t)((debug) & TASK_DBG_MASK_LEVEL) >= TASK_DBG_MIN_LEVEL)) { \
					PLATFORM_DIAG(message); \
				if ((debug) & TASK_DBG_HALT) { \
				while(1); \
			} \
		} \
	} while(0)

#else  /* DEBUG */
#define DEBUG(debug, message)
#endif /* DEBUG */

#endif