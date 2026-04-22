#pragma once
#include <windows.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <utility>
#include "NetworkGraph.h"

namespace ConsoleRenderer {

    constexpr int FIELD_TOP = 3;
    constexpr int FIELD_WIDTH = 80;
    constexpr int FIELD_HEIGHT = 20;

    inline HANDLE handle() {
        return GetStdHandle(STD_OUTPUT_HANDLE);
    }

    inline void gotoXY(int x, int y) {
        COORD c;
        c.X = static_cast<SHORT>(x);
        c.Y = static_cast<SHORT>(y);
        SetConsoleCursorPosition(handle(), c);
    }

    inline void setAttr(WORD attr) {
        SetConsoleTextAttribute(handle(), attr);
    }

    inline void clearField() {
        std::string blank(FIELD_WIDTH, ' ');
        for (int row = FIELD_TOP; row < FIELD_TOP + FIELD_HEIGHT; ++row) {
            gotoXY(0, row);
            std::cout << blank;
        }
    }

    inline void drawLine(int x1, int y1, int x2, int y2, WORD attr) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int adx = std::abs(dx);
        int ady = std::abs(dy);
        int steps = (adx > ady) ? adx : ady;
        if (steps == 0) return;

        char ch;
        if (adx >= ady * 2) ch = '-';
        else if (ady >= adx * 2) ch = '|';
        else if ((dx > 0) == (dy > 0)) ch = '\\';
        else ch = '/';

        setAttr(attr);
        for (int i = 1; i < steps; ++i) {
            double t = static_cast<double>(i) / steps;
            int x = x1 + static_cast<int>(std::round(dx * t));
            int y = y1 + static_cast<int>(std::round(dy * t));
            if (x >= 0 && x < FIELD_WIDTH && y >= 0 && y < FIELD_HEIGHT) {
                gotoXY(x, FIELD_TOP + y);
                std::cout << ch;
            }
        }
    }

    inline void drawWeight(int x1, int y1, int x2, int y2, double weight, WORD attr) {
        int mx = (x1 + x2) / 2;
        int my = (y1 + y2) / 2;
        int offsetY = (std::abs(x2 - x1) >= std::abs(y2 - y1)) ? 1 : 0;
        int sy = FIELD_TOP + my + offsetY;
        int sx = mx + 1;
        if (sy >= FIELD_TOP + FIELD_HEIGHT) sy = FIELD_TOP + FIELD_HEIGHT - 1;
        if (sx < 0) sx = 0;
        if (sx >= FIELD_WIDTH - 4) sx = FIELD_WIDTH - 4;
        setAttr(attr);
        gotoXY(sx, sy);
        int w = static_cast<int>(weight);
        std::cout << w;
    }

    inline void drawNode(int x, int y, int id, WORD attr) {
        std::string s = "[" + std::to_string(id) + "]";
        int screenX = x;
        if (screenX + static_cast<int>(s.size()) > FIELD_WIDTH) {
            screenX = FIELD_WIDTH - static_cast<int>(s.size());
        }
        if (screenX < 0) screenX = 0;
        int screenY = FIELD_TOP + y;
        if (screenY < FIELD_TOP) screenY = FIELD_TOP;
        if (screenY >= FIELD_TOP + FIELD_HEIGHT) screenY = FIELD_TOP + FIELD_HEIGHT - 1;
        setAttr(attr);
        gotoXY(screenX, screenY);
        std::cout << s;
    }

    inline void drawField(const NetworkGraph& graph) {
        clearField();

        const auto& nodes = graph.getNodes();

        std::set<std::pair<int, int>> drawnEdges;
        for (const auto& p : nodes) {
            int u = p.first;
            for (const Edge& e : graph.getNeighbors(u)) {
                int v = e.to;
                auto key = u < v ? std::make_pair(u, v) : std::make_pair(v, u);
                if (drawnEdges.count(key)) continue;
                drawnEdges.insert(key);
                const Node& nu = graph.getNode(u);
                const Node& nv = graph.getNode(v);
                drawLine(nu.x, nu.y, nv.x, nv.y, 8);
                drawWeight(nu.x, nu.y, nv.x, nv.y, e.weight, 14);
            }
        }

        for (const auto& p : nodes) {
            const Node& n = p.second;
            drawNode(n.x, n.y, n.id, 11);
        }

        setAttr(15);
        gotoXY(0, FIELD_TOP + FIELD_HEIGHT);
    }
}
