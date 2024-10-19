// 작성자: bumpsgoodman
// 작성일: 2024-09-13

#include "Precompiled.h"
#include "safe99_Common/Common.h"
#include "safe99_Math/safe99_Math.inl"
#include "Clipping.h"

int __stdcall GetRegion(const int topLeftX, const int topLeftY, const int bottomRightX, const int bottomRightY,
                        const int x, const int y)
{
    int region = REGION_MIDDLE;

    if (x < topLeftX)
    {
        region |= REGION_LEFT;
    }
    else if (x > bottomRightX)
    {
        region |= REGION_RIGHT;
    }

    if (y < topLeftY)
    {
        region |= REGION_TOP;
    }
    else if (y > bottomRightY)
    {
        region |= REGION_BOTTOM;
    }

    return region;
}

bool __stdcall ClipLine(const int topLeftX, const int topLeftY, const int bottomRightX, const int bottomRightY,
                        int* pInOutX0, int* pInOutY0, int* pInOutX1, int* pInOutY1)
{
    ASSERT(pInOutX0 != NULL, "pInOutX0 is NULL");
    ASSERT(pInOutY0 != NULL, "pInOutY0 is NULL");
    ASSERT(pInOutX1 != NULL, "pInOutX1 is NULL");
    ASSERT(pInOutY1 != NULL, "pInOutY1 is NULL");

    int region0 = GetRegion(topLeftX, topLeftY, bottomRightX, bottomRightY, *pInOutX0, *pInOutY0);
    int region1 = GetRegion(topLeftX, topLeftY, bottomRightX, bottomRightY, *pInOutX1, *pInOutY1);

    const int minX = topLeftX;
    const int minY = topLeftY;
    const int maxX = bottomRightX;
    const int maxY = bottomRightY;

    const float slope = (float)(*pInOutY1 - *pInOutY0) / (*pInOutX1 - *pInOutX0);
    const float inverseSlope = (float)(*pInOutX1 - *pInOutX0) / (*pInOutY1 - *pInOutY0);

    // 1. 두 점의 영역이 모두 중앙(화면 안)에 있는 경우 => 클리핑할 필요 없음
    // 2. 두 점의 영역이 모두 화면 밖에 있는 경우 (화면 안을 가로지르지 않는 경우) => 클리핑할 필요 없음(그릴 필요 없음)
    // 3. 두 점의 영역 중 하나라도 화면 안을 가로지르는 경우 => 클리핑 후 재검사
    while (true)
    {
        if ((region0 | region1) == 0)
        {
            return true;
        }
        else if ((region0 & region1) > 0)
        {
            return false;
        }
        else
        {
            int* pRegion;
            int* pX;
            int* pY;
            if (region0 > 0)
            {
                pRegion = &region0;
                pX = pInOutX0;
                pY = pInOutY0;
            }
            else
            {
                pRegion = &region1;
                pX = pInOutX1;
                pY = pInOutY1;
            }

            if ((*pRegion & REGION_LEFT) == REGION_LEFT)
            {
                *pY = ROUND_INT(slope * (minX - *pX) + *pY);
                *pX = minX;
            }
            else if ((*pRegion & REGION_RIGHT) == REGION_RIGHT)
            {
                *pY = ROUND_INT(slope * (maxX - *pX) + *pY);
                *pX = maxX;
            }
            else if ((*pRegion & REGION_TOP) == REGION_TOP)
            {
                *pX = ROUND_INT(inverseSlope * (minY - *pY) + *pX);
                *pY = minY;
            }
            else if ((*pRegion & REGION_BOTTOM) == REGION_BOTTOM)
            {
                *pX = ROUND_INT(inverseSlope * (maxY - *pY) + *pX);
                *pY = maxY;
            }
            else
            {
                ASSERT(false, "Invalid region");
                return false;
            }

            *pRegion = GetRegion(topLeftX, topLeftY, bottomRightX, bottomRightY, *pInOutX0, *pInOutY0);
        }
    }
}