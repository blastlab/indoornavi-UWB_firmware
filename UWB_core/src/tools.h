#ifndef _TOOLS_H
#define _TOOLS_H

#define UNUSED1(a)                  (void)(a)
#define UNUSED2(a,b)                (void)(a),UNUSED1(b)
#define UNUSED3(a,b,c)              (void)(a),UNUSED2(b,c)
#define UNUSED4(a,b,c,d)            (void)(a),UNUSED3(b,c,d)
#define UNUSED5(a,b,c,d,e)          (void)(a),UNUSED4(b,c,d,e)
#define UNUSED6(a,b,c,d,e,f)        (void)(a),UNUSED5(b,c,d,e,f)

#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5, N,...) N
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 5, 4, 3, 2, 1)

#define ALL_UNUSED_IMPL_(nargs) UNUSED ## nargs
#define ALL_UNUSED_IMPL(nargs) ALL_UNUSED_IMPL_(nargs)
#define ALL_UNUSED(...) ALL_UNUSED_IMPL( VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__ )


#define MOD_INCREMENT(VAR,MAX) (VAR=(VAR+1==MAX?0:VAR+1)%MAX)
#define MOD_DECREMENT(VAR,MAX) (VAR=(VAR==0?MAX:VAR)-1)

#endif // _TOOLS_H
