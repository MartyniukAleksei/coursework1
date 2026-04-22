#pragma once
#include "NetworkGraph.h"
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

using namespace std;

struct RoutingResult {
    vector<int> path;
    double totalCost;
    int iterations;
    int relaxations = 0;
    int expandedNodes = 0;
    long long elapsedMicroseconds = 0;
};

struct StepEvent {
    int currentId = -1;
    double currentDist = numeric_limits<double>::infinity();
    vector<int> visited;
    vector<int> frontier;
    int iterations = 0;
    int relaxations = 0;
    int expandedNodes = 0;
    string note;
};

using StepLog = vector<StepEvent>;

class RoutingAlgorithms {
public:
    static RoutingResult dijkstra(const NetworkGraph& graph, int startId, int targetId, StepLog* log = nullptr) {
        RoutingResult result;
        result.totalCost = numeric_limits<double>::infinity();
        result.iterations = 0;

        auto t0 = chrono::high_resolution_clock::now();

        const auto& nodes = graph.getNodes();
        if (nodes.find(startId) == nodes.end() || nodes.find(targetId) == nodes.end()) {
            result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
                chrono::high_resolution_clock::now() - t0).count();
            return result;
        }

        unordered_map<int, double> distances;
        unordered_map<int, int> previous;
        unordered_set<int> settled;

        for (const auto& pair : nodes) {
            distances[pair.first] = numeric_limits<double>::infinity();
        }
        distances[startId] = 0.0;

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
        pq.push({ 0.0, startId });

        while (!pq.empty()) {
            result.iterations++;
            double currentDist = pq.top().first;
            int currentId = pq.top().second;
            pq.pop();

            if (settled.find(currentId) == settled.end()) {
                settled.insert(currentId);
            }

            if (currentDist <= distances[currentId]) {
                for (const Edge& edge : graph.getNeighbors(currentId)) {
                    double newDist = currentDist + edge.weight;

                    if (newDist < distances[edge.to]) {
                        distances[edge.to] = newDist;
                        previous[edge.to] = currentId;
                        result.relaxations++;
                        pq.push({ newDist, edge.to });
                    }
                }
            }

            if (log) {
                StepEvent ev;
                ev.currentId = currentId;
                ev.currentDist = distances[currentId];
                ev.visited.assign(settled.begin(), settled.end());
                for (const auto& p : distances) {
                    if (p.second < numeric_limits<double>::infinity() && settled.find(p.first) == settled.end())
                        ev.frontier.push_back(p.first);
                }
                ev.iterations = result.iterations;
                ev.relaxations = result.relaxations;
                ev.expandedNodes = static_cast<int>(settled.size());
                log->push_back(move(ev));
            }

            if (currentId == targetId) break;
        }

        result.expandedNodes = static_cast<int>(settled.size());

        if (distances[targetId] != numeric_limits<double>::infinity()) {
            result.totalCost = distances[targetId];
            int current = targetId;
            while (current != startId) {
                result.path.push_back(current);
                current = previous[current];
            }
            result.path.push_back(startId);
            reverse(result.path.begin(), result.path.end());
        }

        result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now() - t0).count();
        return result;
    }

    static RoutingResult bellmanFord(const NetworkGraph& graph, int startId, int targetId, StepLog* log = nullptr) {
        RoutingResult result;
        result.totalCost = numeric_limits<double>::infinity();
        result.iterations = 0;

        auto t0 = chrono::high_resolution_clock::now();

        const auto& nodes = graph.getNodes();
        if (nodes.find(startId) == nodes.end() || nodes.find(targetId) == nodes.end()) {
            result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
                chrono::high_resolution_clock::now() - t0).count();
            return result;
        }

        unordered_map<int, double> distances;
        unordered_map<int, int> previous;

        for (const auto& pair : nodes) {
            distances[pair.first] = numeric_limits<double>::infinity();
        }
        distances[startId] = 0.0;

        const int vertexCount = static_cast<int>(nodes.size());

        for (int i = 0; i < vertexCount - 1; ++i) {
            result.iterations++;
            bool anyRelaxation = false;
            vector<int> changedThisPass;
            for (const auto& pair : nodes) {
                int u = pair.first;
                if (distances[u] == numeric_limits<double>::infinity()) continue;
                for (const Edge& edge : graph.getNeighbors(u)) {
                    double newDist = distances[u] + edge.weight;
                    if (newDist < distances[edge.to]) {
                        distances[edge.to] = newDist;
                        previous[edge.to] = u;
                        result.relaxations++;
                        anyRelaxation = true;
                        changedThisPass.push_back(edge.to);
                    }
                }
            }

            if (log) {
                StepEvent ev;
                ev.currentId = -1;
                ev.currentDist = numeric_limits<double>::infinity();
                for (const auto& p : distances) {
                    if (p.second < numeric_limits<double>::infinity()) ev.visited.push_back(p.first);
                }
                ev.frontier = changedThisPass;
                ev.iterations = result.iterations;
                ev.relaxations = result.relaxations;
                ev.expandedNodes = static_cast<int>(ev.visited.size());
                ev.note = "Прохід " + to_string(i + 1) + "/" + to_string(vertexCount - 1);
                log->push_back(move(ev));
            }

            if (!anyRelaxation) break;
        }

        for (const auto& pair : nodes) {
            int u = pair.first;
            if (distances[u] == numeric_limits<double>::infinity()) continue;
            for (const Edge& edge : graph.getNeighbors(u)) {
                if (distances[u] + edge.weight < distances[edge.to]) {
                    result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
                        chrono::high_resolution_clock::now() - t0).count();
                    return result;
                }
            }
        }

        int reached = 0;
        for (const auto& pair : distances) {
            if (pair.second != numeric_limits<double>::infinity()) reached++;
        }
        result.expandedNodes = reached;

        if (distances[targetId] != numeric_limits<double>::infinity()) {
            result.totalCost = distances[targetId];
            int current = targetId;
            while (current != startId) {
                result.path.push_back(current);
                current = previous[current];
            }
            result.path.push_back(startId);
            reverse(result.path.begin(), result.path.end());
        }

        result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now() - t0).count();
        return result;
    }

    static RoutingResult aStar(const NetworkGraph& graph, int startId, int targetId, StepLog* log = nullptr) {
        RoutingResult result;
        result.totalCost = numeric_limits<double>::infinity();
        result.iterations = 0;

        auto t0 = chrono::high_resolution_clock::now();

        const auto& nodes = graph.getNodes();
        if (nodes.find(startId) == nodes.end() || nodes.find(targetId) == nodes.end()) {
            result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
                chrono::high_resolution_clock::now() - t0).count();
            return result;
        }

        const Node& targetNode = graph.getNode(targetId);

        unordered_map<int, double> gScore;
        unordered_map<int, int> previous;
        unordered_set<int> closed;

        for (const auto& pair : nodes) {
            gScore[pair.first] = numeric_limits<double>::infinity();
        }
        gScore[startId] = 0.0;

        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> open;
        open.push({ euclideanHeuristic(graph.getNode(startId), targetNode), startId });

        bool found = false;
        while (!open.empty()) {
            result.iterations++;
            int currentId = open.top().second;
            open.pop();

            if (closed.find(currentId) != closed.end()) continue;
            closed.insert(currentId);

            bool isTarget = (currentId == targetId);
            if (!isTarget) {
                for (const Edge& edge : graph.getNeighbors(currentId)) {
                    if (closed.find(edge.to) != closed.end()) continue;
                    double tentativeG = gScore[currentId] + edge.weight;
                    if (tentativeG < gScore[edge.to]) {
                        gScore[edge.to] = tentativeG;
                        previous[edge.to] = currentId;
                        result.relaxations++;
                        double f = tentativeG + euclideanHeuristic(graph.getNode(edge.to), targetNode);
                        open.push({ f, edge.to });
                    }
                }
            }

            if (log) {
                StepEvent ev;
                ev.currentId = currentId;
                ev.currentDist = gScore[currentId];
                ev.visited.assign(closed.begin(), closed.end());
                for (const auto& p : gScore) {
                    if (p.second < numeric_limits<double>::infinity() && closed.find(p.first) == closed.end())
                        ev.frontier.push_back(p.first);
                }
                ev.iterations = result.iterations;
                ev.relaxations = result.relaxations;
                ev.expandedNodes = static_cast<int>(closed.size());
                log->push_back(move(ev));
            }

            if (isTarget) {
                found = true;
                break;
            }
        }

        result.expandedNodes = static_cast<int>(closed.size());

        if (found) {
            result.totalCost = gScore[targetId];
            int current = targetId;
            while (current != startId) {
                result.path.push_back(current);
                current = previous[current];
            }
            result.path.push_back(startId);
            reverse(result.path.begin(), result.path.end());
        }

        result.elapsedMicroseconds = chrono::duration_cast<chrono::microseconds>(
            chrono::high_resolution_clock::now() - t0).count();
        return result;
    }

private:
    static double euclideanHeuristic(const Node& a, const Node& b) {
        double dx = static_cast<double>(a.x - b.x);
        double dy = static_cast<double>(a.y - b.y);
        return sqrt(dx * dx + dy * dy);
    }
};
