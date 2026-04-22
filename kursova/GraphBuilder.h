#pragma once
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <sstream>
#include <string>
#include "NetworkGraph.h"
#include "ConsoleRenderer.h"

namespace GraphBuilder {

    namespace detail {
        constexpr int K_UP = 72;
        constexpr int K_DOWN = 80;
        constexpr int K_LEFT = 75;
        constexpr int K_RIGHT = 77;
        constexpr int K_ENTER = 13;
        constexpr int K_ESC = 27;

        inline void setAttr(WORD a) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), a);
        }
    }

    inline NetworkGraph placeNodesInteractive() {
        NetworkGraph g;
        int cursorX = 10;
        int cursorY = 5;
        int nextId = 1;

        while (true) {
            system("cls");
            detail::setAttr(10);
            std::cout << "=== РУЧНА ПОБУДОВА: РОЗМІЩЕННЯ ВУЗЛІВ ===\n";
            detail::setAttr(15);
            std::cout << "Стрілки - перемістити курсор. Enter - додати вузол у позицію. ESC - завершити.\n\n";

            ConsoleRenderer::drawField(g);

            ConsoleRenderer::gotoXY(0, ConsoleRenderer::FIELD_TOP + ConsoleRenderer::FIELD_HEIGHT + 1);
            detail::setAttr(14);
            std::cout << "Курсор: (" << cursorX << ", " << cursorY << ")    Вузлів: "
                      << g.getNodes().size() << "    Наступний ID: " << nextId << "   ";
            detail::setAttr(15);

            ConsoleRenderer::gotoXY(cursorX, ConsoleRenderer::FIELD_TOP + cursorY);
            detail::setAttr((10 << 4) | 0);
            std::cout << "+";
            detail::setAttr(15);

            int k = _getch();
            if (k == 224) {
                k = _getch();
                if (k == detail::K_UP && cursorY > 0) cursorY--;
                else if (k == detail::K_DOWN && cursorY < ConsoleRenderer::FIELD_HEIGHT - 1) cursorY++;
                else if (k == detail::K_LEFT && cursorX > 0) cursorX--;
                else if (k == detail::K_RIGHT && cursorX < ConsoleRenderer::FIELD_WIDTH - 4) cursorX++;
            }
            else if (k == detail::K_ENTER) {
                g.addNode(nextId, "R" + std::to_string(nextId), cursorX, cursorY);
                nextId++;
            }
            else if (k == detail::K_ESC) {
                break;
            }
        }
        return g;
    }

    inline void addEdgesByText(NetworkGraph& g) {
        system("cls");
        detail::setAttr(10);
        std::cout << "=== РУЧНА ПОБУДОВА: ВВЕДЕННЯ РЕБЕР ===\n";
        detail::setAttr(15);
        std::cout << "Формат рядка: <id_from> <id_to> <weight>. Порожній рядок - завершити.\n";
        std::cout << "Доступні ID вузлів: ";
        for (const auto& p : g.getNodes()) std::cout << p.first << " ";
        std::cout << "\n\n";

        std::cin.clear();
        std::cin.sync();

        while (true) {
            std::cout << "> ";
            std::string line;
            if (!std::getline(std::cin, line)) break;
            if (line.empty()) break;
            std::istringstream iss(line);
            int u, v;
            double w;
            if (!(iss >> u >> v >> w)) {
                detail::setAttr(12);
                std::cout << "  Невірний формат. Очікую: <id_from> <id_to> <weight>\n";
                detail::setAttr(15);
                continue;
            }
            const auto& nodes = g.getNodes();
            if (nodes.find(u) == nodes.end() || nodes.find(v) == nodes.end()) {
                detail::setAttr(12);
                std::cout << "  Один із ID не існує.\n";
                detail::setAttr(15);
                continue;
            }
            g.addEdge(u, v, w);
            detail::setAttr(10);
            std::cout << "  Ребро " << u << " <-> " << v << " (" << w << " мс) додано.\n";
            detail::setAttr(15);
        }
    }

    inline NetworkGraph buildInteractive() {
        NetworkGraph g = placeNodesInteractive();
        if (g.getNodes().empty()) return g;
        addEdgesByText(g);
        return g;
    }
}
