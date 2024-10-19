// 작성자: bumpsgoodman
// 작성일: 2024-09-13

#ifndef SAFE99_CLIPPING_H
#define SAFE99_CLIPPING_H

typedef enum REGION
{
    REGION_MIDDLE = 0x00,   // 0b0000
    REGION_LEFT =   0x01,   // 0b0001
    REGION_RIGHT =  0x02,   // 0b0010
    REGION_BOTTOM = 0x04,   // 0b0100
    REGION_TOP =    0x08,   // 0b1000
} REGION;

int     __stdcall   GetRegion(const int topLeftX, const int topLeftY, const int bottomRightX, const int bottomRightY,
                              const int x, const int y);

// 영역 안에 있다면 true, 아니라면 false
bool    __stdcall   ClipLine(const int topLeftX, const int topLeftY, const int bottomRightX, const int bottomRightY,
                             int* pInOutX0, int* pInOutY0, int* pInOutX1, int* pInOutY1);

#endif // SAFE99_CLIPPING_H