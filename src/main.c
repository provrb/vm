#include "api/lexer.h"
#include <stdio.h>
#include <windows.h>

int main() {
    // Start timer
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER startTime;
    QueryPerformanceCounter(&startTime);

    ParseTokens("./test.pvb");

    LARGE_INTEGER endTime;
    QueryPerformanceCounter(&endTime);

    double elapsedTime =
        (double)(endTime.QuadPart - startTime.QuadPart) / frequency.QuadPart * 1000.0;

    printf("Function execution time: %.3f ms\n", elapsedTime);

    return 0;
}
