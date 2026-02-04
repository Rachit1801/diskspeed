#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ull unsigned long long

#define FILE_SIZE (1024ULL * 1024 * 1024)
#define CHUNK_SIZE (4 * 1024 * 1024)


double getTime() {
    LARGE_INTEGER f, t;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart / f.QuadPart;
}

void fillRandom(char *buf, ull size) {
    for (ull i = 0; i < size; i++)
        buf[i] = rand() % 256;
}

int main() {

    srand(time(NULL));

    HANDLE hFile;
    DWORD written, read;

    char *buffer = (char*)VirtualAlloc(
        NULL,
        CHUNK_SIZE,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (!buffer) {
        printf("Alloc failed\n");
        return 1;
    }

    fillRandom(buffer, CHUNK_SIZE);

    /* WRITE */
    hFile = CreateFileA(
        "speed_test.bin",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("CreateFile write failed\n");
        return 1;
    }

    ull loops = FILE_SIZE / CHUNK_SIZE;

    double start = getTime();

    for (ull i = 0; i < loops; i++) {
        WriteFile(hFile, buffer, CHUNK_SIZE, &written, NULL);
    }

    CloseHandle(hFile);

    double end = getTime();

    printf("Real Write: %.2f MB/s\n",
        (FILE_SIZE / (1024.0 * 1024)) / (end - start));


    /* READ */
    hFile = CreateFileA(
        "speed_test.bin",
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("CreateFile read failed\n");
        return 1;
    }

    start = getTime();

    for (ull i = 0; i < loops; i++) {
        ReadFile(hFile, buffer, CHUNK_SIZE, &read, NULL);
    }

    CloseHandle(hFile);

    end = getTime();

    printf("Real Read: %.2f MB/s\n",
        (FILE_SIZE / (1024.0 * 1024)) / (end - start));


    VirtualFree(buffer, 0, MEM_RELEASE);

    DeleteFileA("speed_test.bin");

    return 0;
}

#endif
