#include <Windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <vector>

#include "generic/fixed_vector.h"
#include "generic/dynamic_vector.h"
#include "util/assert.h"

//#include "Source.h"

using namespace std;

int main(void)
{
    fixed_vector_t v0;
    vector<uint64_t> v1;
    dynamic_vector_t v2;

    fixed_vector_init(&v0, sizeof(uint64_t), 5000000);
    dynamic_vector_init(&v2, sizeof(uint64_t), 5000000);
    v1.reserve(5000000);

    //test12();

    srand((unsigned int)time(NULL));

    LARGE_INTEGER frequency;
    LARGE_INTEGER prev_counter;
    LARGE_INTEGER cur_counter;

    QueryPerformanceFrequency(&frequency);

    {
        printf("fixed_vector test\n");
        QueryPerformanceCounter(&prev_counter);
        for (size_t i = 0; i < 5000000; ++i)
        {
            //size_t r = rand() % SIZE_MAX;
            fixed_vector_push_back(&v0, &i, sizeof(uint64_t));
            //fixed_vector_insert(&v0, &i, sizeof(uint64_t), 0);
        }
        QueryPerformanceCounter(&cur_counter);
        double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
        float elapsed_tick = (float)d_elapsed_tick;
        printf("insert: %.4fms\n\n", elapsed_tick);
    }

    {
        printf("std::vector test\n");
        QueryPerformanceCounter(&prev_counter);
        for (size_t i = 0; i < 5000000; ++i)
        {
            v1.push_back(i);
            //v1.insert(v1.begin() + rand() % (i + 1), rand() % SIZE_MAX);
            //v1.insert(v1.begin(), i);
        }
        QueryPerformanceCounter(&cur_counter);
        double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
        float elapsed_tick = (float)d_elapsed_tick;
        printf("insert: %.4fms\n\n", elapsed_tick);
    }
    
    {
        printf("dynamic_vector test\n");
        QueryPerformanceCounter(&prev_counter);
        for (size_t i = 0; i < 5000000; ++i)
        {
            //size_t r = rand() % SIZE_MAX;
            dynamic_vector_push_back(&v2, &i, sizeof(uint64_t));
            //dynamic_vector_insert(&v2, &r, sizeof(uint64_t), rand() % (i + 1));
        }
        QueryPerformanceCounter(&cur_counter);
        double d_elapsed_tick = ((double)cur_counter.QuadPart - (double)prev_counter.QuadPart) / (double)frequency.QuadPart * 1000.0;
        float elapsed_tick = (float)d_elapsed_tick;
        printf("insert: %.4fms\n\n", elapsed_tick);
    }

    return 0;
}