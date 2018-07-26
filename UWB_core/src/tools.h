/**
 * @brief file for small code snippets
 * 
 * @file tools.h
 * @author Karol Trzcinski
 * @date 2018-07-02
 * 
 */
#ifndef _TOOLS_H
#define _TOOLS_H


#define UNUSED_1(a)                  (void)(a)
#define UNUSED_2(a,b)                (void)(a),UNUSED_1(b)
#define UNUSED_3(a,b,c)              (void)(a),UNUSED_2(b,c)
#define UNUSED_4(a,b,c,d)            (void)(a),UNUSED_3(b,c,d)
#define UNUSED_5(a,b,c,d,e)          (void)(a),UNUSED_4(b,c,d,e)
#define UNUSED_6(a,b,c,d,e,f)        (void)(a),UNUSED_5(b,c,d,e,f)
#define UNUSED_7(a,b,c,d,e,f,g)      (void)(a),UNUSED_6(b,c,d,e,f,g)
#define UNUSED_8(a,b,c,d,e,f,g,h)    (void)(a),UNUSED_7(b,c,d,e,f,g,h)

#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9, N,...) N
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define ALL_UNUSED_IMPL_(nargs) UNUSED_ ## nargs
#define ALL_UNUSED_IMPL(nargs) ALL_UNUSED_IMPL_(nargs)


/**
 * @brief useful to prevent compilator 'unused variable' warning
 * 
 */
#define ALL_UNUSED(...) ALL_UNUSED_IMPL( VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__ )


/**
 * @brief increment value and trim to <start,stop) range
 * 
 */
#define INCREMENT_CYCLE(VAR,START,STOP) (VAR=((VAR+1)==STOP?START:(VAR+1)))


/**
 * @brief decrement value and trim to <start,stop) range
 * 
 */
#define DECREMENT_CYCLE(VAR,START,STOP) (VAR=(VAR==(START?STOP:VAR)-1))


/**
 * @brief increment value and trim to <0,max) range
 * 
 */
#define INCREMENT_MOD(VAR,MAX) INCREMENT_CYCLE(VAR,0,MAX)


/**
 * @brief decrement value and trim to <0,max) range
 * 
 */
#define DECREMENT_MOD(VAR,MAX) DECREMENT_CYCLE(VAR,0,MAX)

#endif // _TOOLS_H
