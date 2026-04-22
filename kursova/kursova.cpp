#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <windows.h>

#include "NetworkGraph.h"
#include "RoutingAlgorithms.h"
#include "ConsoleRenderer.h"
#include "TopologyLibrary.h"
#include "GraphBuilder.h"
#include "AlgorithmVisualizer.h"

using namespace std;

const int KEY_UP = 72;
const int KEY_DOWN = 80;
const int KEY_ENTER = 13;

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}


void setColor(int textColor, int bgColor = 0) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    int colorAttribute = (bgColor << 4) | textColor;
    SetConsoleTextAttribute(consoleHandle, colorAttribute);
}


void waitKey(const string& prompt = "Натисніть будь-яку клавішу для повернення...") {
    cout << prompt;
    _getch();
}

void renderGraphScreen(const NetworkGraph& g, const string& title) {
    system("cls");
    setColor(10);
    cout << "=== " << title << " ===\n";
    setColor(15);
    cout << "Вузлів: " << g.getNodes().size() << ".   ";
    ConsoleRenderer::drawField(g);
    ConsoleRenderer::gotoXY(0, ConsoleRenderer::FIELD_TOP + ConsoleRenderer::FIELD_HEIGHT + 1);
    waitKey("Натисніть будь-яку клавішу, щоб обрати маршрут...");
}

void selectEndpointsAndPreview(const NetworkGraph& g) {
    if (g.getNodes().size() < 2) {
        system("cls");
        setColor(12);
        cout << "У графі менше двох вузлів — вибір маршруту неможливий.\n";
        setColor(15);
        waitKey();
        return;
    }

    system("cls");
    setColor(10);
    cout << "=== ВИБІР МАРШРУТУ ===\n\n";
    setColor(15);
    cout << "Доступні вузли:\n";
    for (const auto& p : g.getNodes()) {
        cout << "  [" << p.first << "] " << p.second.name
             << "  (x=" << p.second.x << ", y=" << p.second.y << ")\n";
    }
    cout << "\n";

    int startId = 0, targetId = 0;
    cin.clear();
    cin.sync();

    cout << "Введіть ID стартового вузла: ";
    if (!(cin >> startId)) {
        cin.clear(); cin.ignore(10000, '\n');
        setColor(12); cout << "Невірний ввід.\n"; setColor(15);
        waitKey(); return;
    }
    cout << "Введіть ID цільового вузла:  ";
    if (!(cin >> targetId)) {
        cin.clear(); cin.ignore(10000, '\n');
        setColor(12); cout << "Невірний ввід.\n"; setColor(15);
        waitKey(); return;
    }
    cin.ignore(10000, '\n');

    const auto& nodes = g.getNodes();
    if (nodes.find(startId) == nodes.end() || nodes.find(targetId) == nodes.end()) {
        setColor(12);
        cout << "\nОдин із ID не існує в графі.\n";
        setColor(15);
        waitKey(); return;
    }
    if (startId == targetId) {
        setColor(12);
        cout << "\nСтартовий і цільовий вузли збігаються.\n";
        setColor(15);
        waitKey(); return;
    }

    setColor(14);
    cout << "\nОбрано маршрут: [" << startId << "] " << g.getNode(startId).name
         << "  ->  [" << targetId << "] " << g.getNode(targetId).name << "\n";
    setColor(15);

    cout << "\nЗараз буде запущено візуалізацію трьох алгоритмів:\n";
    setColor(11);
    cout << "  Дейкстра  ->  Беллман-Форд  ->  A*\n";
    setColor(8);
    cout << "Керування під час анімації: Space — пауза, Up/Down — швидкість, Esc — пропустити крок.\n";
    setColor(15);
    cout << "\n";
    waitKey("Натисніть будь-яку клавішу, щоб почати...");

    AlgorithmVisualizer::runAllAlgorithms(g, startId, targetId);
}

void finalizeGraph(const NetworkGraph& g, const string& title) {
    if (g.getNodes().empty()) return;
    renderGraphScreen(g, title);
    selectEndpointsAndPreview(g);
}

void showTopologySelection() {
    vector<string> items = {
        "Star (зірка, 6 вузлів)",
        "Mesh (сітка, 5 вузлів)",
        "Linear (ланцюг, 6 вузлів)",
        "Назад"
    };
    int activeIndex = 0;
    bool inMenu = true;

    while (inMenu) {
        system("cls");
        setColor(10);
        cout << "=== ЗАГОТОВЛЕНІ ТОПОЛОГІЇ ===\n\n";
        setColor(15);
        cout << "Оберіть топологію для перегляду:\n\n";

        for (size_t i = 0; i < items.size(); ++i) {
            if (static_cast<int>(i) == activeIndex) {
                setColor(0, 15);
                cout << " -> " << items[i] << " \n";
                setColor(15, 0);
            }
            else {
                cout << "    " << items[i] << " \n";
            }
        }

        int key = _getch();
        if (key == 224) {
            key = _getch();
            if (key == KEY_UP) {
                activeIndex--;
                if (activeIndex < 0) activeIndex = static_cast<int>(items.size()) - 1;
            }
            else if (key == KEY_DOWN) {
                activeIndex++;
                if (activeIndex >= static_cast<int>(items.size())) activeIndex = 0;
            }
        }
        else if (key == KEY_ENTER) {
            switch (activeIndex) {
            case 0: finalizeGraph(TopologyLibrary::makeStar(), "ТОПОЛОГІЯ: STAR"); break;
            case 1: finalizeGraph(TopologyLibrary::makeMesh(), "ТОПОЛОГІЯ: MESH"); break;
            case 2: finalizeGraph(TopologyLibrary::makeLinear(), "ТОПОЛОГІЯ: LINEAR"); break;
            case 3: inMenu = false; break;
            }
        }
    }
}

void showPlay() {
    vector<string> items = {
        "Побудувати граф вручну",
        "Обрати заготовлену топологію",
        "Назад"
    };
    int activeIndex = 0;
    bool inPlay = true;

    while (inPlay) {
        system("cls");
        setColor(10);
        cout << "=== СИМУЛЯЦІЯ МЕРЕЖІ ===\n\n";
        setColor(15);
        cout << "Оберіть спосіб побудови графа маршрутизаторів:\n\n";

        for (size_t i = 0; i < items.size(); ++i) {
            if (static_cast<int>(i) == activeIndex) {
                setColor(0, 15);
                cout << " -> " << items[i] << " \n";
                setColor(15, 0);
            }
            else {
                cout << "    " << items[i] << " \n";
            }
        }

        int key = _getch();
        if (key == 224) {
            key = _getch();
            if (key == KEY_UP) {
                activeIndex--;
                if (activeIndex < 0) activeIndex = static_cast<int>(items.size()) - 1;
            }
            else if (key == KEY_DOWN) {
                activeIndex++;
                if (activeIndex >= static_cast<int>(items.size())) activeIndex = 0;
            }
        }
        else if (key == KEY_ENTER) {
            switch (activeIndex) {
            case 0: {
                NetworkGraph g = GraphBuilder::buildInteractive();
                finalizeGraph(g, "ГРАФ ПОБУДОВАНО ВРУЧНУ");
                break;
            }
            case 1:
                showTopologySelection();
                break;
            case 2:
                inPlay = false;
                break;
            }
        }
    }
}

void showHelp() {
    system("cls");
    setColor(10);
    cout << "=== ДОВІДКА ===\n\n";
    setColor(15);

    auto row = [](const string& key, const string& action) {
        cout << "  ";
        setColor(14);
        cout << key;
        setColor(15);
        int pad = 28 - static_cast<int>(key.size());
        if (pad < 1) pad = 1;
        cout << string(pad, ' ') << "| " << action << "\n";
    };
    auto section = [](const string& title) {
        cout << "\n";
        setColor(11);
        cout << "  [" << title << "]\n";
        setColor(15);
        cout << "  " << string(28, '-') << "+-" << string(45, '-') << "\n";
    };

    section("Меню");
    row("Up / Down",             "навігація між пунктами");
    row("Enter",                 "обрати активний пункт");

    section("Ручна побудова графа");
    row("Up / Down / Left / Right", "перемістити курсор по 2D-полю");
    row("Enter",                 "додати вузол у поточну позицію");
    row("Esc",                   "завершити розміщення вузлів");
    row("<id> <id> <вага>",      "у текстовому режимі: додати ребро");
    row("(порожній рядок)",      "завершити введення ребер");

    section("Вибір маршруту");
    row("<число> Enter",         "ввести ID стартового, потім цільового вузла");

    section("Візуалізація алгоритмів");
    row("Space",                 "пауза / продовжити анімацію");
    row("Up / Down",             "збільшити / зменшити швидкість (1-5)");
    row("Esc",                   "пропустити поточний алгоритм");

    cout << "\n";
    waitKey();
}

void showAbout() {
    system("cls");
    setColor(10);
    cout << "=== ПРО ПРОГРАМУ ===\n\n";
    setColor(15);
    cout << "Курсовий проєкт: симулятор порівняння алгоритмів маршрутизації\n";
    cout << "на графі мережі маршрутизаторів (Дейкстра, Беллман-Форд, A*).\n\n";

    setColor(11);
    cout << "Функціонал:\n";
    setColor(15);
    cout << "  - побудова графа вручну або з бібліотеки заготовлених топологій\n";
    cout << "  - покрокова візуалізація на 2D-полі з керованою швидкістю\n";
    cout << "  - анімація проходження пакета обраним шляхом\n";
    cout << "  - послідовний прогін трьох алгоритмів на одному графі\n";
    cout << "  - порівняльний звіт з метриками (ітерації, релаксації,\n";
    cout << "    розкриті вузли, час виконання) і висновком\n\n";

    setColor(11);
    cout << "Технології:\n";
    setColor(15);
    cout << "  C++17, Windows Console API (<windows.h>, <conio.h>),\n";
    cout << "  MSBuild / Visual Studio 2022, кодова сторінка CP1251.\n\n";

    waitKey();
}


int main() {

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    hideCursor();

    vector<string> menuItems = { "Play (Симуляція)", "Help (Довідка)", "About (Про програму)", "Exit (Вихід)" };
    int activeIndex = 0;
    bool running = true;

    while (running) {
        system("cls"); 


        setColor(10);
        cout << "=== МЕРЕЖЕВИЙ СИМУЛЯТОР ===\n\n";
        setColor(15);

        const int itemCount = static_cast<int>(menuItems.size());
        for (int i = 0; i < itemCount; ++i) {
            if (i == activeIndex) {
                setColor(0, 15);
                cout << " -> " << menuItems[i] << " \n";
                setColor(15, 0);
            }
            else {
                cout << "    " << menuItems[i] << " \n";
            }
        }

        int key = _getch();

        if (key == 224) {
            key = _getch();
            if (key == KEY_UP) {
                activeIndex--;
                if (activeIndex < 0) activeIndex = itemCount - 1;
            }
            else if (key == KEY_DOWN) {
                activeIndex++;
                if (activeIndex >= itemCount) activeIndex = 0;
            }
        }
        else if (key == KEY_ENTER) {
            switch (activeIndex) {
            case 0: showPlay(); break;
            case 1: showHelp(); break;
            case 2: showAbout(); break;
            case 3: running = false; break;
            }
        }
    }

    system("cls");
    setColor(10);
    cout << "Роботу завершено!\n";
    setColor(15);

    return 0;
}