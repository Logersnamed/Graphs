#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "vertex.h"
#include "edge.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

enum EventName {
    SET_START_VERTEX,
    SET_CURRENT_VERTEX,
    SET_END_VERTEX,
    CHECK_VERTEX,
    CHECK_EDGE,
    UNCHECK_VERTEX,
    UNCHECK_EDGE,
    SET_WEIGHT
};

struct Event {
    EventName name;
    int vertexId;
    int edgeId;
    qreal weight;
};

typedef std::unordered_map<int, Vertex*> vertexMap;
typedef std::unordered_map<int, Edge*> edgeMap;
typedef std::vector<Event> Events;

class Dijkstra {
public:
    Dijkstra();

    Events run(Vertex &startVertex, vertexMap vertices, edgeMap edges);

private:
    Events events;

    std::vector<int> dUnchecked;
    std::vector<int> dCheckedEdges;
    std::vector<int> dCheckedVertices;
    int dCurrVertex = -1;

    void logEvent(EventName name, int vertexId, int edgeId, qreal weight);
    void weightsToInf(Vertex& startVertex, vertexMap vertices, std::vector<int>& unchecked);
    void dijkstraAlgorithm(Vertex &startVertex, vertexMap vertices, edgeMap edges, std::vector<int>& checked);
};

#endif // DIJKSTRA_H
