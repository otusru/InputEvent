#include <windows.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>

struct InputEvent {
    DWORD delay;        // Задержка в миллисекундах
    POINT position;     // Положение курсора
    DWORD eventType;    // 1 - клавиатура, 2 - мышь
    DWORD keyCode;      // Код клавиши или события мыши
};

// === Глобальные переменные ===
std::vector<InputEvent> recordedEvents;
std::atomic<bool> stopRecording(false);

// === Обработчики хуков ===
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static DWORD lastTime = GetTickCount();
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)lParam;
        DWORD now = GetTickCount();
        recordedEvents.push_back({ now - lastTime, ms->pt, 2, wParam });
        lastTime = now;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static DWORD lastTime = GetTickCount();
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* ks = (KBDLLHOOKSTRUCT*)lParam;
        DWORD now = GetTickCount();
        recordedEvents.push_back({ now - lastTime, {0, 0}, 1, ks->vkCode });
        lastTime = now;

        // Остановка записи по F10
        if (wParam == WM_KEYDOWN && ks->vkCode == VK_F10) {
            stopRecording = true;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// === Сохранение в файл ===
void SaveToFile(const std::string& filename) {
    std::ofstream out(filename);
    for (auto& e : recordedEvents) {
        out << "[" << e.delay << "];"
            << "{" << e.position.x << ", " << e.position.y << "};"
            << "[" << e.eventType << "];"
            << "(" << e.keyCode << ");\n";
    }
}

// === Загрузка из файла ===
std::vector<InputEvent> LoadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    std::vector<InputEvent> events;
    std::string line;

    while (std::getline(in, line)) {
        InputEvent e;
        sscanf(line.c_str(), "[%lu];{%ld, %ld};[%lu];(%lu);",
               &e.delay, &e.position.x, &e.position.y, &e.eventType, &e.keyCode);
        events.push_back(e);
    }
    return events;
}

// === Воспроизведение событий ===
void PlayEvents(const std::vector<InputEvent>& events, bool loop) {
    std::cout << "Начинается воспроизведение. Нажмите F10 для остановки...\n";
    do {
        for (const auto& e : events) {
            std::this_thread::sleep_for(std::chrono::milliseconds(e.delay));

            if (GetAsyncKeyState(VK_F10)) {
                std::cout << "Воспроизведение остановлено.\n";
                return;
            }

            if (e.eventType == 1) { // Клавиатура
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = e.keyCode;
                input.ki.dwFlags = KEYEVENTF_KEYDOWN;
                SendInput(1, &input, sizeof(INPUT));

                // Отпускание клавиши
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
            } else if (e.eventType == 2) { // Мышь
                SetCursorPos(e.position.x, e.position.y);

                INPUT input = {};
                input.type = INPUT_MOUSE;
                switch (e.keyCode) {
                    case WM_LBUTTONDOWN: input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; break;
                    case WM_LBUTTONUP:   input.mi.dwFlags = MOUSEEVENTF_LEFTUP;   break;
                    case WM_RBUTTONDOWN: input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; break;
                    case WM_RBUTTONUP:   input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;   break;
                }
                SendInput(1, &input, sizeof(INPUT));
            }
        }
    } while (loop);
}

// === Главная функция ===
int main() {
    std::cout << "=== Макрорекордер на C++ ===\n";
    std::cout << "1 - Запись действий\n2 - Воспроизведение\nВыбор: ";
    int mode;
    std::cin >> mode;

    if (mode == 1) {
        std::cout << "Запись началась. Нажмите F10 для остановки.\n";

        HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
        HHOOK keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

        // Ожидание сигнала остановки
        MSG msg;
        while (!stopRecording && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        UnhookWindowsHookEx(mouseHook);
        UnhookWindowsHookEx(keyHook);

        SaveToFile("actions.txt");
        std::cout << "Запись завершена и сохранена в actions.txt\n";
    }
    else if (mode == 2) {
        auto events = LoadFromFile("actions.txt");
        if (events.empty()) {
            std::cout << "Файл пуст или не найден.\n";
            return 1;
        }

        std::cout << "Повторять воспроизведение? (1 - да, 0 - нет): ";
        bool repeat;
        std::cin >> repeat;

        PlayEvents(events, repeat);
    }

    return 0;
}
