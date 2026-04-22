#pragma once
#include "NetworkGraph.h"

namespace TopologyLibrary {

    inline NetworkGraph makeStar() {
        NetworkGraph g;
        g.addNode(1, "R1", 40, 10);
        g.addNode(2, "R2", 15, 3);
        g.addNode(3, "R3", 65, 3);
        g.addNode(4, "R4", 15, 17);
        g.addNode(5, "R5", 65, 17);
        g.addNode(6, "R6", 75, 10);
        g.addEdge(1, 2, 5);
        g.addEdge(1, 3, 7);
        g.addEdge(1, 4, 6);
        g.addEdge(1, 5, 8);
        g.addEdge(1, 6, 4);
        return g;
    }

    inline NetworkGraph makeMesh() {
        NetworkGraph g;
        g.addNode(1, "R1", 10, 4);
        g.addNode(2, "R2", 40, 2);
        g.addNode(3, "R3", 70, 4);
        g.addNode(4, "R4", 20, 16);
        g.addNode(5, "R5", 55, 17);
        g.addEdge(1, 2, 10);
        g.addEdge(2, 3, 8);
        g.addEdge(1, 4, 6);
        g.addEdge(3, 5, 5);
        g.addEdge(4, 5, 12);
        g.addEdge(2, 4, 15);
        g.addEdge(2, 5, 9);
        g.addEdge(1, 3, 20);
        return g;
    }

    inline NetworkGraph makeLinear() {
        NetworkGraph g;
        g.addNode(1, "R1", 5, 10);
        g.addNode(2, "R2", 19, 10);
        g.addNode(3, "R3", 33, 10);
        g.addNode(4, "R4", 47, 10);
        g.addNode(5, "R5", 61, 10);
        g.addNode(6, "R6", 75, 10);
        g.addEdge(1, 2, 3);
        g.addEdge(2, 3, 7);
        g.addEdge(3, 4, 5);
        g.addEdge(4, 5, 2);
        g.addEdge(5, 6, 8);
        return g;
    }

}
