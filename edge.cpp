#include "edge.h"
#include "canvas.h"

Edge::Edge(int edgeId, int fristId, int secodnId, qreal weight, QWidget* parent)
    : id(edgeId), startId(fristId), endId(secodnId), weight(weight) {

}
