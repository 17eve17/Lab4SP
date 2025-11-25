// Завдання 2.1: Читання великого файлу стандартними функціями C
#include <stdio.h>
#include <time.h>

#define BUFFER_SIZE 8192

int main() {
    FILE* file = NULL;
    // Використовуємо безпечний fopen_s
    errno_t err = fopen_s(&file, "C:\\Users\\eve\\Desktop\\largefile.dat", "rb");
    if (err != 0 || file == NULL) {
        printf("Не вдалося відкрити файл.\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    clock_t start = clock();
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        // Тут можна обробляти дані
    }
    clock_t end = clock();
    printf("Час читання (fread_s): %.2f секунд\n", (double)(end - start) / CLOCKS_PER_SEC);

    fclose(file);
    return 0;
}
