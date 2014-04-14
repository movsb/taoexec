#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef _DEBUG
#define debug_out(x) DebugOut##x
#else
#define debug_out(x) 
#endif

#ifdef __cplusplus
extern "C"{
#endif

void DebugOut(char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif//!DEBUG_H_
