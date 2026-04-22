#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

struct Edge {
    int to;
    double weight;
};

struct Node {
    int id;
    string name;
    int x, y;
};

class NetworkGraph {
private:
    unordered_map<int, Node> nodes;

    unordered_map<int, vector<Edge>> adjList;

public:
    void addNode(int id, const string& name, int x = 0, int y = 0) {
        nodes[id] = { id, name, x, y };
        if (adjList.find(id) == adjList.end()) {
            adjList[id] = vector<Edge>();
        }
    }

    void addEdge(int u, int v, double weight, bool isDirected = false) {
        adjList[u].push_back({ v, weight });
        if (!isDirected) {
            adjList[v].push_back({ u, weight });
        }
    }


    const unordered_map<int, Node>& getNodes() const { return nodes; }

    const vector<Edge>& getNeighbors(int id) const { return adjList.at(id); }

    const Node& getNode(int id) const { return nodes.at(id); }
};
