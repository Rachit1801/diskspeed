#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ull unsigned long long

#define MAX_FILE_SIZE (1024ULL * 1024 * 1024) // 1GB
#define CHUNK_SIZE    (4 * 1024 * 1024)       // 4MB
#define MAX_TIME      10.0                   // 10 seconds

/* High precision timer */
double getTime() {
    LARGE_INTEGER f, t;
    QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart / f.QuadPart;
}

/* Fill random once */
void fillRandom(char *buf, ull size) {
    for (ull i = 0; i < size; i++)
        buf[i] = rand() % 256;
}

/* Progress Bar */
void showProgress(double doneMB, double totalMB,
                  double speed) {

    int width = 30;

    double percent = doneMB / totalMB;

    int filled = (int)(percent * width);

    printf("\r[");

    for (int i = 0; i < width; i++) {
        if (i < filled) printf("#");
        else printf(" ");
    }

    printf("] %5.1f%% | %7.1f MB/s",
           percent * 100,
           speed);

    fflush(stdout);
}


int main() {

    srand(time(NULL));

    printf("=== Pro Disk Benchmark v2 ===\n\n");

    HANDLE hFile;
    DWORD done;

    char *buffer = (char*)VirtualAlloc(
        NULL,
        CHUNK_SIZE,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (!buffer) {
        printf("Memory alloc failed\n");
        return 1;
    }

    fillRandom(buffer, CHUNK_SIZE);

    ull maxLoops = MAX_FILE_SIZE / CHUNK_SIZE;
    ull loops = 0;

    double start, now, elapsed;

    double totalMB = MAX_FILE_SIZE / (1024.0 * 1024);


    /* ================= WRITE ================= */

    printf("WRITE TEST\n");

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
        printf("Write open failed\n");
        return 1;
    }

    start = getTime();

    while (loops < maxLoops) {

        WriteFile(hFile, buffer, CHUNK_SIZE, &done, NULL);

        loops++;

        now = getTime();
        elapsed = now - start;

        double doneMB =
            (loops * CHUNK_SIZE) / (1024.0 * 1024);

        double speed = doneMB / elapsed;

        showProgress(doneMB, totalMB, speed);

        if (elapsed >= MAX_TIME)
            break;
    }

    CloseHandle(hFile);

    elapsed = getTime() - start;

    double finalWrite =
        ((loops * CHUNK_SIZE) / (1024.0 * 1024)) / elapsed;

    printf("\nFinal Write Speed: %.2f MB/s\n\n", finalWrite);


    /* ================= READ ================= */

    printf("READ TEST\n");

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
        printf("Read open failed\n");
        return 1;
    }

    loops = 0;

    start = getTime();

    while (loops < maxLoops) {

        ReadFile(hFile, buffer, CHUNK_SIZE, &done, NULL);

        loops++;

        now = getTime();
        elapsed = now - start;

        double doneMB =
            (loops * CHUNK_SIZE) / (1024.0 * 1024);

        double speed = doneMB / elapsed;

        showProgress(doneMB, totalMB, speed);

        if (elapsed >= MAX_TIME)
            break;
    }

    CloseHandle(hFile);

    elapsed = getTime() - start;

    double finalRead =
        ((loops * CHUNK_SIZE) / (1024.0 * 1024)) / elapsed;

    printf("\nFinal Read Speed: %.2f MB/s\n\n", finalRead);


    VirtualFree(buffer, 0, MEM_RELEASE);

    DeleteFileA("speed_test.bin");

    printf("Benchmark Done ✅\n");

    return 0;
}

#endif
