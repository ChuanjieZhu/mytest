/*
	$Id: pqlog.h, v1.0.0, 2011.6.28, YellowBug $
	$@@: http下载 
*/

#ifndef PENQ_LOG_H_
#define PENQ_LOG_H_

#define PQL_DEBUG		1
#define PQL_ERROR		2
#define PQL_NECESSARY	3

#if defined(PQL_DEBUG)
#define PQ_LOGD(...)	printf("D/(%s, %d) ", __FILE__, __LINE__);printf(__VA_ARGS__)
#else
#define PQ_LOGD(...)
#endif

#if defined(PQL_ERROR)
#define PQ_LOGE(...)	printf("E/(%s, %d) ", __FILE__, __LINE__);printf(__VA_ARGS__)
#else
#define PQ_LOGE(...)
#endif

#if defined(PQL_NECESSARY)
#define PQ_LOGN(...)	printf("N/(%s, %d) ", __FILE__, __LINE__);printf(__VA_ARGS__)
#else
#define PQ_LOGN(...)
#endif


#endif /* #ifndef PENQ_LOG_H_ */

/* END */

