#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <aclapi.h>
#include <time.h>

#define BUFFER_SIZE 8192

// ---------------------- Завдання 1 ----------------------
void Task1() {
    LPCTSTR filename = TEXT("C:\\Users\\eve\\Desktop\\example.txt");

    DWORD attributes = GetFileAttributes(filename);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        _tprintf(TEXT("Не вдалося отримати атрибути файлу. Помилка %lu\n"), GetLastError());
        return;
    }

    _tprintf(TEXT("Атрибути файлу: "));
    if (attributes & FILE_ATTRIBUTE_ARCHIVE) _tprintf(TEXT("Archive "));
    if (attributes & FILE_ATTRIBUTE_HIDDEN) _tprintf(TEXT("Hidden "));
    if (attributes & FILE_ATTRIBUTE_READONLY) _tprintf(TEXT("ReadOnly "));
    if (attributes & FILE_ATTRIBUTE_SYSTEM) _tprintf(TEXT("System "));
    _tprintf(TEXT("\n"));

    HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("Не вдалося відкрити файл. Помилка %lu\n"), GetLastError());
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    _tprintf(TEXT("Розмір файлу: %lu байт\n"), fileSize);

    FILETIME creationTime, accessTime, writeTime;
    if (GetFileTime(hFile, &creationTime, &accessTime, &writeTime)) {
        SYSTEMTIME stUTC, stLocal;

        FileTimeToSystemTime(&creationTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        _tprintf(TEXT("Час створення: %02d-%02d-%04d %02d:%02d:%02d\n"),
            stLocal.wDay, stLocal.wMonth, stLocal.wYear,
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

        FileTimeToSystemTime(&accessTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        _tprintf(TEXT("Час останнього доступу: %02d-%02d-%04d %02d:%02d:%02d\n"),
            stLocal.wDay, stLocal.wMonth, stLocal.wYear,
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

        FileTimeToSystemTime(&writeTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
        _tprintf(TEXT("Час останньої модифікації: %02d-%02d-%04d %02d:%02d:%02d\n"),
            stLocal.wDay, stLocal.wMonth, stLocal.wYear,
            stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
    }

    PSECURITY_DESCRIPTOR pSD = NULL;
    PSID pOwnerSID = NULL;
    if (GetSecurityInfo(hFile, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
        &pOwnerSID, NULL, NULL, NULL, &pSD) == ERROR_SUCCESS) {
        TCHAR name[256], domain[256];
        DWORD nameSize = 256, domainSize = 256;
        SID_NAME_USE sidType;
        if (LookupAccountSid(NULL, pOwnerSID, name, &nameSize, domain, &domainSize, &sidType)) {
            _tprintf(TEXT("Власник: %s\\%s\n"), domain, name);
        }
        LocalFree(pSD);
    }

    CloseHandle(hFile);
}

// ---------------------- Завдання 2.1 ----------------------
void Task2_1() {
    FILE* file = NULL;
    errno_t err = _wfopen_s(&file, TEXT("C:\\Users\\eve\\Desktop\\largefile.dat"), TEXT("rb"));
    if (err != 0 || file == NULL) {
        _tprintf(TEXT("Не вдалося відкрити файл.\n"));
        return;
    }

    char buffer[BUFFER_SIZE];
    clock_t start = clock();
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        // Обробка даних
    }
    clock_t end = clock();
    _tprintf(TEXT("Час читання (fread_s): %.2f секунд\n"), (double)(end - start) / CLOCKS_PER_SEC);

    fclose(file);
}

// ---------------------- Завдання 2.2 ----------------------
void Task2_2() {
    HANDLE hFile = CreateFile(TEXT("C:\\Users\\eve\\Desktop\\largefile.dat"), GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        _tprintf(TEXT("Не вдалося відкрити файл. Помилка %lu\n"), GetLastError());
        return;
    }

    char buffer[BUFFER_SIZE];
    DWORD bytesRead;
    clock_t start = clock();
    while (ReadFile(hFile, buffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
        // Обробка даних
    }
    clock_t end = clock();
    _tprintf(TEXT("Час читання (ReadFile): %.2f секунд\n"), (double)(end - start) / CLOCKS_PER_SEC);

    CloseHandle(hFile);
}

// ---------------------- Завдання 3 ----------------------
void Task3() {
    const TCHAR* files[3] = {
        TEXT("C:\\Users\\eve\\Desktop\\file1.dat"),
        TEXT("C:\\Users\\eve\\Desktop\\file2.dat"),
        TEXT("C:\\Users\\eve\\Desktop\\file3.dat")
    };

    HANDLE hFiles[3];
    OVERLAPPED overlaps[3];
    char buffers[3][BUFFER_SIZE];
    HANDLE events[3];

    for (int i = 0; i < 3; ++i) {
        events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hFiles[i] = CreateFile(files[i], GENERIC_READ, 0, NULL, OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED, NULL);
        ZeroMemory(&overlaps[i], sizeof(OVERLAPPED));
        overlaps[i].hEvent = events[i];
        ReadFile(hFiles[i], buffers[i], BUFFER_SIZE, NULL, &overlaps[i]);
    }

    int remaining = 3;
    while (remaining > 0) {
        DWORD finished = WaitForMultipleObjects(remaining, events, FALSE, INFINITE);
        int idx = finished - WAIT_OBJECT_0;

        DWORD bytesRead;
        if (GetOverlappedResult(hFiles[idx], &overlaps[idx], &bytesRead, FALSE)) {
            _tprintf(TEXT("Файл %s прочитано %lu байт\n"), files[idx], bytesRead);
        }

        CloseHandle(hFiles[idx]);
        CloseHandle(events[idx]);

        for (int j = idx; j < remaining - 1; ++j) {
            hFiles[j] = hFiles[j + 1];
            overlaps[j] = overlaps[j + 1];
            events[j] = events[j + 1];
            memcpy(buffers[j], buffers[j + 1], BUFFER_SIZE);
        }
        remaining--;
    }
}

// ---------------------- main ----------------------
int _tmain() {
    _tprintf(TEXT("----- Завдання 1 -----\n"));
    Task1();

    _tprintf(TEXT("\n----- Завдання 2.1 -----\n"));
    Task2_1();

    _tprintf(TEXT("\n----- Завдання 2.2 -----\n"));
    Task2_2();

    _tprintf(TEXT("\n----- Завдання 3 -----\n"));
    Task3();

    return 0;
}
