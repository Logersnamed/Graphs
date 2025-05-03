#ifndef VERTEX_H
#define VERTEX_H

#include <QWidget.h>

typedef std::unordered_map<int, int> intMap;

class Vertex {

public:
    Vertex(QString displayName, int id, int radius, QPointF& pos, QWidget* parent = nullptr);

    QString displayName;
    int id;
    int radius;
    QPointF pos;

    int weight = -2;

    struct {
        std::vector<int> vertexId;
        std::vector<int> edgeId;
    } in, out;
};

#endif // VERTEX_H
