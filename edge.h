#ifndef EDGE_H
#define EDGE_H

#include <QWidget>

class Edge {

public:
    Edge(int edgeId, int fristId, int secodnId, int weight, QWidget* parent = nullptr);

    int id;

    int startId;
    int endId;

    int weight;
};

#endif // EDGE_H
