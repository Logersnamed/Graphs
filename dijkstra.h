#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "vertex.h"
#include "edge.h"

#include <unordered_map>
#include <vector>

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
    static Events run(Vertex &startVertex, vertexMap vertices, edgeMap edges);

private:
    static void logEvent(Events &events, EventName name, int vertexId, int edgeId, qreal weight);
    static void weightsToInf(Vertex& startVertex, vertexMap vertices, std::vector<int>& unchecked, Events &events);
    static int dijkstraAlgorithm(Vertex &startVertex, vertexMap vertices, edgeMap edges,
                          std::vector<int>& checked, std::vector<int>& checkedEdges,
                          std::vector<int>& unchecked, Events &events);
};

#endif // DIJKSTRA_H
