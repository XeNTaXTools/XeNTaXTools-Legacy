// TriStripTest.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <boost/lambda/lambda.hpp>
#include <string.h>
#include <boost/shared_array.hpp>


#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;

typedef unsigned char  BYTE; // 1byte
typedef unsigned short  WORD; // 2bytes
typedef unsigned long  DWORD; //4bytes
typedef unsigned int uint32;
typedef unsigned short uint16;



const WORD* src ;


struct TriangleList
{
    boost::shared_array<WORD> data;
    DWORD n_indices;
    DWORD triangles;
} dst;

bool ConvertStripCutToTriangleList(const uint16* src, uint32 n, TriangleList& dst, uint16 terminal = 0xFFFF)
{
// validate
    if(!src || !n) return false;
    if(n < 3) return false;

// compute number of triangles
    uint32 n_triangles = 0;
    uint32 a = 0;
    uint32 b = 0;
    for(;;)
    {
        // must stop
        if(b == n)
        {
            if(src[n - 1] != terminal)
            {
                uint32 distance = b - a;
                if(distance < 3) return false;
                n_triangles += distance - 2;
            }
            break;
        }

        // hit terminal index
        if(src[b] == terminal)
        {
            uint32 distance = b - a;
            if(distance < 3) return false;
            n_triangles += distance - 2;
            a = b + 1;
            b = a;
        }
        // non-terminal index
        else
            b++;
    }

// initialize destination data
    dst.triangles = n_triangles;
    dst.n_indices = n_triangles * 3;
    dst.data.reset(new uint16[dst.n_indices]);

// initialize second pass data
    uint16* ptr = dst.data.get();
    uint32 dst_index = 0;
    uint32 src_index = 0;

// second pass through source
    do
    {

        // keep track of tristrip index
        uint32 tristrip_index = 0;

        // copy first triangle
        uint16 a = src[src_index++];
        uint16 b = src[src_index++];
        uint16 c = src[src_index++];
        ptr[dst_index++] = a;
        ptr[dst_index++] = b;
        ptr[dst_index++] = c;
        tristrip_index++;

        // copy other triangles
        while(src_index < n)
        {
            a = b;
            b = c;
            c = src[src_index++];
            if(c == terminal) break;
            if(tristrip_index % 2)
            {
                ptr[dst_index++] = a;
                ptr[dst_index++] = c;
                ptr[dst_index++] = b;
            }
            else
            {
                ptr[dst_index++] = a;
                ptr[dst_index++] = b;
                ptr[dst_index++] = c;
            }
            tristrip_index++;
        }
    }
    while(src_index < n);

    return true;
}







void testStrip()
{

    boost::shared_array<WORD> dst.data(new WORD[65535]);
    ConvertStripCutToTriangleList(src, 4417, dst, 0xFFFF) ;   // 4417= face indices count


}
int main()
{

    testStrip();
    system("pause");
    return 0;
}


