#ifndef EDGE_H
#define EDGE_H

#include <QWidget>

class Edge {

public:
    Edge(int edgeId, int fristId, int secodnId, qreal weight, QWidget* parent = nullptr);

    int id;

    int startId;
    int endId;

    qreal weight;
};

#endif // EDGE_H
