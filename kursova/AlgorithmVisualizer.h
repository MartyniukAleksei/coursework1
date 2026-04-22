#pragma once
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <limits>

#include "NetworkGraph.h"
#include "RoutingAlgorithms.h"
#include "ConsoleRenderer.h"

namespace AlgorithmVisualizer {

    constexpr int PANEL_TOP = ConsoleRenderer::FIELD_TOP + ConsoleRenderer::FIELD_HEIGHT;
    constexpr int PANEL_HEIGHT = 3;

    constexpr WORD COLOR_EDGE_DEFAULT  = 8;
    constexpr WORD COLOR_EDGE_PATH     = 10;
    constexpr WORD COLOR_WEIGHT        = 14;
    constexpr WORD COLOR_NODE_DEFAULT  = 11;
    constexpr WORD COLOR_NODE_VISITED  = 2;
    constexpr WORD COLOR_NODE_FRONTIER = 13;
    constexpr WORD COLOR_NODE_CURRENT  = 14;
    constexpr WORD COLOR_NODE_START    = 9;
    constexpr WORD COLOR_NODE_TARGET   = 12;
    constexpr WORD COLOR_NODE_PATH     = 10;
    constexpr WORD COLOR_PACKET        = 14;
    constexpr WORD COLOR_TEXT          = 15;
    constexpr WORD COLOR_HINT          = 8;
    constexpr WORD COLOR_HEADER        = 11;

    struct SpeedState {
        int level = 3;
        bool paused = false;
        bool skip = false;
    };

    inline int delayMsForLevel(int level) {
        switch (level) {
            case 1: return 900;
            case 2: return 450;
            case 3: return 220;
            case 4: return 110;
            default: return 55;
        }
    }

    inline std::string speedBar(int level) {
        std::string s = "[";
        for (int i = 1; i <= 5; ++i) s += (i <= level ? '#' : '.');
        s += "] ";
        s += std::to_string(level);
        s += "/5";
        return s;
    }

    inline void clearPanel() {
        std::string blank(ConsoleRenderer::FIELD_WIDTH, ' ');
        for (int i = 0; i < PANEL_HEIGHT; ++i) {
            ConsoleRenderer::gotoXY(0, PANEL_TOP + i);
            std::cout << blank;
        }
    }

    inline void drawInfoPanel(const std::string& algoName, const SpeedState& ss,
                              const StepEvent* ev, const NetworkGraph& g) {
        clearPanel();

        ConsoleRenderer::setAttr(COLOR_HEADER);
        ConsoleRenderer::gotoXY(0, PANEL_TOP);
        std::cout << "Алгоритм: " << algoName;

        ConsoleRenderer::setAttr(COLOR_TEXT);
        ConsoleRenderer::gotoXY(30, PANEL_TOP);
        std::cout << "Статус: " << (ss.paused ? "PAUSED " : "PLAYING");

        ConsoleRenderer::gotoXY(52, PANEL_TOP);
        std::cout << "Швидкість: " << speedBar(ss.level);

        ConsoleRenderer::gotoXY(0, PANEL_TOP + 1);
        if (ev) {
            if (ev->currentId >= 0 && g.getNodes().count(ev->currentId)) {
                std::cout << "Поточний: " << g.getNode(ev->currentId).name;
                ConsoleRenderer::gotoXY(18, PANEL_TOP + 1);
                std::cout << "Відстань: ";
                if (ev->currentDist >= std::numeric_limits<double>::infinity()) std::cout << "inf";
                else std::cout << ev->currentDist;
            } else if (!ev->note.empty()) {
                std::cout << ev->note;
            }
            ConsoleRenderer::gotoXY(40, PANEL_TOP + 1);
            std::cout << "Iter: " << ev->iterations
                      << "  Relax: " << ev->relaxations
                      << "  Expand: " << ev->expandedNodes;
        }

        ConsoleRenderer::setAttr(COLOR_HINT);
        ConsoleRenderer::gotoXY(0, PANEL_TOP + 2);
        std::cout << "Space - пауза | Up/Down - швидкість | Esc - пропустити";
        ConsoleRenderer::setAttr(COLOR_TEXT);
    }

    inline void pollInput(SpeedState& ss) {
        while (_kbhit()) {
            int key = _getch();
            if (key == 224 || key == 0) {
                int k2 = _getch();
                if (k2 == 72) { if (ss.level < 5) ss.level++; }
                else if (k2 == 80) { if (ss.level > 1) ss.level--; }
            } else if (key == ' ') {
                ss.paused = !ss.paused;
            } else if (key == 27) {
                ss.skip = true;
            } else if (key == '+' || key == '=') {
                if (ss.level < 5) ss.level++;
            } else if (key == '-' || key == '_') {
                if (ss.level > 1) ss.level--;
            }
        }
    }

    inline void sleepResponsive(int ms, SpeedState& ss,
                                const std::string& algoName,
                                const StepEvent* ev, const NetworkGraph& g) {
        using namespace std::chrono;
        auto start = steady_clock::now();
        int lastLevel = ss.level;
        bool lastPaused = ss.paused;
        while (true) {
            pollInput(ss);
            if (ss.skip) return;
            if (ss.level != lastLevel || ss.paused != lastPaused) {
                drawInfoPanel(algoName, ss, ev, g);
                lastLevel = ss.level;
                lastPaused = ss.paused;
            }
            if (ss.paused) {
                Sleep(30);
                continue;
            }
            auto elapsed = duration_cast<milliseconds>(steady_clock::now() - start).count();
            if (elapsed >= ms) return;
            Sleep(15);
        }
    }

    inline WORD nodeColorFor(int id, const StepEvent& ev,
                             const std::unordered_set<int>& visited,
                             const std::unordered_set<int>& frontier,
                             int startId, int targetId) {
        if (id == startId) return COLOR_NODE_START;
        if (id == targetId) return COLOR_NODE_TARGET;
        if (id == ev.currentId) return COLOR_NODE_CURRENT;
        if (visited.count(id)) return COLOR_NODE_VISITED;
        if (frontier.count(id)) return COLOR_NODE_FRONTIER;
        return COLOR_NODE_DEFAULT;
    }

    inline void repaintStep(const NetworkGraph& g, const StepEvent& ev,
                            int startId, int targetId) {
        std::unordered_set<int> visited(ev.visited.begin(), ev.visited.end());
        std::unordered_set<int> frontier(ev.frontier.begin(), ev.frontier.end());
        for (const auto& p : g.getNodes()) {
            int id = p.first;
            WORD attr = nodeColorFor(id, ev, visited, frontier, startId, targetId);
            ConsoleRenderer::drawNode(p.second.x, p.second.y, id, attr);
        }
    }

    inline void highlightPath(const NetworkGraph& g, const std::vector<int>& path,
                              int startId, int targetId) {
        if (path.size() < 2) return;
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            const Node& a = g.getNode(path[i]);
            const Node& b = g.getNode(path[i + 1]);
            ConsoleRenderer::drawLine(a.x, a.y, b.x, b.y, COLOR_EDGE_PATH);
            for (const Edge& e : g.getNeighbors(path[i])) {
                if (e.to == path[i + 1]) {
                    ConsoleRenderer::drawWeight(a.x, a.y, b.x, b.y, e.weight, COLOR_WEIGHT);
                    break;
                }
            }
        }
        for (int id : path) {
            const Node& n = g.getNode(id);
            WORD attr = COLOR_NODE_PATH;
            if (id == startId) attr = COLOR_NODE_START;
            if (id == targetId) attr = COLOR_NODE_TARGET;
            ConsoleRenderer::drawNode(n.x, n.y, id, attr);
        }
    }

    inline char edgeCharFor(int dx, int dy) {
        int adx = std::abs(dx), ady = std::abs(dy);
        if (adx >= ady * 2) return '-';
        if (ady >= adx * 2) return '|';
        if ((dx > 0) == (dy > 0)) return '\\';
        return '/';
    }

    inline void animatePacket(const NetworkGraph& g, const std::vector<int>& path,
                              SpeedState& ss, const std::string& algoName) {
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            if (ss.skip) return;
            const Node& a = g.getNode(path[i]);
            const Node& b = g.getNode(path[i + 1]);
            int dx = b.x - a.x, dy = b.y - a.y;
            int adx = std::abs(dx), ady = std::abs(dy);
            int steps = (adx > ady) ? adx : ady;
            if (steps < 1) continue;
            char edgeChar = edgeCharFor(dx, dy);
            int lastX = -1, lastY = -1;
            for (int step = 1; step < steps; ++step) {
                pollInput(ss);
                if (ss.skip) {
                    ConsoleRenderer::drawLine(a.x, a.y, b.x, b.y, COLOR_EDGE_PATH);
                    return;
                }
                double t = static_cast<double>(step) / steps;
                int x = a.x + static_cast<int>(std::round(dx * t));
                int y = a.y + static_cast<int>(std::round(dy * t));
                if (lastX >= 0) {
                    ConsoleRenderer::setAttr(COLOR_EDGE_PATH);
                    ConsoleRenderer::gotoXY(lastX, ConsoleRenderer::FIELD_TOP + lastY);
                    std::cout << edgeChar;
                }
                ConsoleRenderer::setAttr(COLOR_PACKET);
                ConsoleRenderer::gotoXY(x, ConsoleRenderer::FIELD_TOP + y);
                std::cout << '*';
                lastX = x; lastY = y;
                int d = (std::max)(30, delayMsForLevel(ss.level) / 3);
                sleepResponsive(d, ss, algoName, nullptr, g);
                if (ss.skip) {
                    ConsoleRenderer::drawLine(a.x, a.y, b.x, b.y, COLOR_EDGE_PATH);
                    return;
                }
            }
            if (lastX >= 0) {
                ConsoleRenderer::setAttr(COLOR_EDGE_PATH);
                ConsoleRenderer::gotoXY(lastX, ConsoleRenderer::FIELD_TOP + lastY);
                std::cout << edgeChar;
            }
            ConsoleRenderer::drawNode(b.x, b.y, b.id,
                (b.id == path.back() ? COLOR_NODE_TARGET : COLOR_NODE_PATH));
        }
        ConsoleRenderer::setAttr(COLOR_TEXT);
    }

    inline void paintBaseline(const NetworkGraph& g, int startId, int targetId) {
        for (const auto& p : g.getNodes()) {
            WORD attr = COLOR_NODE_DEFAULT;
            if (p.first == startId) attr = COLOR_NODE_START;
            else if (p.first == targetId) attr = COLOR_NODE_TARGET;
            ConsoleRenderer::drawNode(p.second.x, p.second.y, p.first, attr);
        }
    }

    using AlgoFn = RoutingResult(*)(const NetworkGraph&, int, int, StepLog*);

    inline RoutingResult runSingle(const std::string& algoName, AlgoFn fn,
                                   const NetworkGraph& g, int startId, int targetId,
                                   SpeedState& ss) {
        ss.skip = false;

        StepLog log;
        RoutingResult res = fn(g, startId, targetId, &log);

        system("cls");
        ConsoleRenderer::setAttr(COLOR_EDGE_PATH);
        std::cout << "=== ВІЗУАЛІЗАЦІЯ: " << algoName << " ===\n";
        ConsoleRenderer::setAttr(COLOR_TEXT);
        std::cout << "Start: [" << startId << "] " << g.getNode(startId).name
                  << "   Target: [" << targetId << "] " << g.getNode(targetId).name << "\n";

        ConsoleRenderer::drawField(g);
        paintBaseline(g, startId, targetId);

        const StepEvent* lastEv = nullptr;
        drawInfoPanel(algoName, ss, lastEv, g);

        for (size_t i = 0; i < log.size(); ++i) {
            repaintStep(g, log[i], startId, targetId);
            lastEv = &log[i];
            drawInfoPanel(algoName, ss, lastEv, g);
            sleepResponsive(delayMsForLevel(ss.level), ss, algoName, lastEv, g);
            if (ss.skip) break;
        }

        if (!res.path.empty()) {
            highlightPath(g, res.path, startId, targetId);
        }
        drawInfoPanel(algoName, ss, lastEv, g);

        if (ss.skip) {
            ss.skip = false;
        } else if (!res.path.empty() && res.path.size() > 1) {
            animatePacket(g, res.path, ss, algoName);
        }
        ss.skip = false;

        return res;
    }

    struct MetricRow {
        std::string label;
        std::string values[3];
        bool isValid[3] = { false, false, false };
        double numerics[3] = { 0.0, 0.0, 0.0 };
    };

    inline std::string formatDouble1(double v) {
        char buf[64];
        sprintf_s(buf, sizeof(buf), "%.1f", v);
        return buf;
    }

    inline std::string padRightStr(const std::string& s, int width) {
        if ((int)s.size() >= width) return s;
        return s + std::string(width - (int)s.size(), ' ');
    }

    inline void printSeparator(int labelW, const int colW[3]) {
        std::cout << "+" << std::string(labelW + 2, '-')
                  << "+" << std::string(colW[0] + 2, '-')
                  << "+" << std::string(colW[1] + 2, '-')
                  << "+" << std::string(colW[2] + 2, '-')
                  << "+\n";
    }

    inline void renderComparisonReport(
        const std::string algoNames[3],
        const RoutingResult results[3],
        const NetworkGraph& g, int startId, int targetId)
    {
        std::vector<MetricRow> rows;

        auto fillIntRow = [&](const std::string& label, auto getter) {
            MetricRow r;
            r.label = label;
            for (int i = 0; i < 3; ++i) {
                if (results[i].path.empty()) {
                    r.values[i] = "-";
                    r.isValid[i] = false;
                } else {
                    long long v = getter(results[i]);
                    r.values[i] = std::to_string(v);
                    r.isValid[i] = true;
                    r.numerics[i] = static_cast<double>(v);
                }
            }
            rows.push_back(r);
        };

        auto fillDoubleRow = [&](const std::string& label, auto getter) {
            MetricRow r;
            r.label = label;
            for (int i = 0; i < 3; ++i) {
                if (results[i].path.empty()) {
                    r.values[i] = "-";
                    r.isValid[i] = false;
                } else {
                    double v = getter(results[i]);
                    r.values[i] = formatDouble1(v);
                    r.isValid[i] = true;
                    r.numerics[i] = v;
                }
            }
            rows.push_back(r);
        };

        fillIntRow("Довжина",   [](const RoutingResult& r){ return (long long)(r.path.size() - 1); });
        fillDoubleRow("Вартість", [](const RoutingResult& r){ return r.totalCost; });
        fillIntRow("Ітерацій",   [](const RoutingResult& r){ return (long long)r.iterations; });
        fillIntRow("Релаксацій", [](const RoutingResult& r){ return (long long)r.relaxations; });
        fillIntRow("Розкрито",   [](const RoutingResult& r){ return (long long)r.expandedNodes; });
        fillIntRow("Час, мкс",   [](const RoutingResult& r){ return r.elapsedMicroseconds; });

        int labelW = (int)std::string("Метрика").size();
        for (const auto& row : rows) labelW = (std::max)(labelW, (int)row.label.size());

        int colW[3] = { 0, 0, 0 };
        for (int i = 0; i < 3; ++i) {
            colW[i] = (std::max)(colW[i], (int)algoNames[i].size());
            for (const auto& row : rows) colW[i] = (std::max)(colW[i], (int)row.values[i].size());
        }

        system("cls");
        ConsoleRenderer::setAttr(COLOR_EDGE_PATH);
        std::cout << "=== ПОРІВНЯЛЬНИЙ ЗВІТ ===\n\n";
        ConsoleRenderer::setAttr(COLOR_TEXT);
        std::cout << "Маршрут: " << g.getNode(startId).name
                  << " -> " << g.getNode(targetId).name << "\n\n";

        printSeparator(labelW, colW);
        std::cout << "| " << padRightStr("Метрика", labelW) << " ";
        for (int i = 0; i < 3; ++i) {
            std::cout << "| " << padRightStr(algoNames[i], colW[i]) << " ";
        }
        std::cout << "|\n";
        printSeparator(labelW, colW);

        int wins[3] = { 0, 0, 0 };

        for (const auto& row : rows) {
            double bestVal = 0, worstVal = 0;
            bool anyValid = false;
            for (int i = 0; i < 3; ++i) {
                if (!row.isValid[i]) continue;
                if (!anyValid) { bestVal = worstVal = row.numerics[i]; anyValid = true; }
                else {
                    if (row.numerics[i] < bestVal)  bestVal  = row.numerics[i];
                    if (row.numerics[i] > worstVal) worstVal = row.numerics[i];
                }
            }
            bool allSame = anyValid && (bestVal == worstVal);

            std::cout << "| " << padRightStr(row.label, labelW) << " ";
            for (int i = 0; i < 3; ++i) {
                std::cout << "| ";
                if (row.isValid[i] && !allSame) {
                    if (row.numerics[i] == bestVal) ConsoleRenderer::setAttr(10);
                    else if (row.numerics[i] == worstVal) ConsoleRenderer::setAttr(12);
                }
                std::cout << padRightStr(row.values[i], colW[i]);
                ConsoleRenderer::setAttr(COLOR_TEXT);
                std::cout << " ";
            }
            std::cout << "|\n";

            if (anyValid && !allSame) {
                std::vector<int> winners;
                for (int i = 0; i < 3; ++i) {
                    if (row.isValid[i] && row.numerics[i] == bestVal) winners.push_back(i);
                }
                if (winners.size() == 1) wins[winners[0]]++;
            }
        }
        printSeparator(labelW, colW);

        std::cout << "\nПереможці за метриками:\n";
        for (const auto& row : rows) {
            std::cout << "  " << padRightStr(row.label + ":", labelW + 1) << " ";
            double bestVal = 0;
            bool anyValid = false;
            for (int i = 0; i < 3; ++i) {
                if (!row.isValid[i]) continue;
                if (!anyValid) { bestVal = row.numerics[i]; anyValid = true; }
                else if (row.numerics[i] < bestVal) bestVal = row.numerics[i];
            }
            if (!anyValid) { std::cout << "-\n"; continue; }
            std::vector<int> winners;
            for (int i = 0; i < 3; ++i) {
                if (row.isValid[i] && row.numerics[i] == bestVal) winners.push_back(i);
            }
            if ((int)winners.size() == 3) {
                std::cout << "рівні (" << row.values[winners[0]] << ")\n";
            } else {
                ConsoleRenderer::setAttr(10);
                for (size_t k = 0; k < winners.size(); ++k) {
                    std::cout << algoNames[winners[k]];
                    if (k + 1 < winners.size()) std::cout << ", ";
                }
                ConsoleRenderer::setAttr(COLOR_TEXT);
                std::cout << " (" << row.values[winners[0]] << ")\n";
            }
        }

        bool anyFound = false;
        for (int i = 0; i < 3; ++i) if (!results[i].path.empty()) anyFound = true;

        std::cout << "\n";
        if (!anyFound) {
            ConsoleRenderer::setAttr(COLOR_NODE_TARGET);
            std::cout << "Висновок: жоден з алгоритмів не знайшов шлях між обраними вузлами.\n";
            ConsoleRenderer::setAttr(COLOR_TEXT);
        } else {
            int topIdx = 0;
            for (int i = 1; i < 3; ++i) if (wins[i] > wins[topIdx]) topIdx = i;
            int total = (int)rows.size();
            std::cout << "Висновок: ";
            ConsoleRenderer::setAttr(COLOR_WEIGHT);
            std::cout << "найефективнішим на цьому графі виявився " << algoNames[topIdx]
                      << " (лідер за " << wins[topIdx] << "/" << total << " метриками).";
            ConsoleRenderer::setAttr(COLOR_TEXT);
            std::cout << "\n";
        }

        std::cout << "\nШляхи:\n";
        int pathLabelW = 0;
        for (int i = 0; i < 3; ++i)
            pathLabelW = (std::max)(pathLabelW, (int)algoNames[i].size() + 1);
        for (int i = 0; i < 3; ++i) {
            std::cout << "  " << padRightStr(algoNames[i] + ":", pathLabelW) << " ";
            if (results[i].path.empty()) {
                ConsoleRenderer::setAttr(COLOR_HINT);
                std::cout << "не знайдено";
                ConsoleRenderer::setAttr(COLOR_TEXT);
            } else {
                for (size_t k = 0; k < results[i].path.size(); ++k) {
                    std::cout << g.getNode(results[i].path[k]).name;
                    if (k + 1 < results[i].path.size()) std::cout << " -> ";
                }
            }
            std::cout << "\n";
        }

        std::cout << "\nНатисніть будь-яку клавішу для повернення...";
        _getch();
    }

    inline void runAllAlgorithms(const NetworkGraph& g, int startId, int targetId) {
        SpeedState ss;

        struct Algo {
            std::string name;
            AlgoFn fn;
        };
        Algo algos[3] = {
            { "Дейкстра",    &RoutingAlgorithms::dijkstra    },
            { "Беллман-Форд", &RoutingAlgorithms::bellmanFord },
            { "A*",          &RoutingAlgorithms::aStar       }
        };

        std::vector<RoutingResult> results(3);
        for (int i = 0; i < 3; ++i) {
            results[i] = runSingle(algos[i].name, algos[i].fn, g, startId, targetId, ss);
            if (i < 2) {
                ConsoleRenderer::setAttr(COLOR_WEIGHT);
                ConsoleRenderer::gotoXY(0, PANEL_TOP + PANEL_HEIGHT - 1);
                std::string msg = "Далі: " + algos[i + 1].name + ". Натисніть будь-яку клавішу...";
                std::string pad(ConsoleRenderer::FIELD_WIDTH - (int)msg.size(), ' ');
                std::cout << msg << pad;
                ConsoleRenderer::setAttr(COLOR_TEXT);
                _getch();
            }
        }

        std::string names[3] = { algos[0].name, algos[1].name, algos[2].name };
        RoutingResult resArr[3] = { results[0], results[1], results[2] };
        renderComparisonReport(names, resArr, g, startId, targetId);
    }
}
