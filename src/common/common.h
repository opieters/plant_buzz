#ifndef __COMMON_H__
#define __COMMON_H__

template< typename T > T& rom_get( int idx, T &t );
template< typename T > const T& rom_put( int idx, const T &t );

typedef struct {short x; short y;} Location; 

#endif