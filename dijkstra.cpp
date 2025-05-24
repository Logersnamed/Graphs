#include "utils.h"
#include "dijkstra.h"

#define INF -1
#define x -1

Dijkstra::Dijkstra() {}

void Dijkstra::logEvent(Events &events, EventName name, int vertexId, int edgeId, qreal weight) {
    events.push_back({
        name,
        vertexId,
        edgeId,
        weight
    });
}

int getMinWeightVertex(std::vector<int> vertexIds, vertexMap vertices) {
    int minId = vertexIds[0];
    qreal minWeight = vertices.at(minId)->weight;

    for (int id : vertexIds) {
        qreal weight = vertices.at(id)->weight;

        if (weight != INF && weight < minWeight) {
            minWeight = weight;
            minId = id;
        }
    }

    return minId;
}

void Dijkstra::weightsToInf(Vertex& startVertex, vertexMap vertices, std::vector<int>& unchecked, Events &events) {
    unchecked.push_back(startVertex.id);
    startVertex.weight = INF;
    logEvent(events, SET_WEIGHT, startVertex.id, x, INF);

    for (int id : startVertex.out.vertexId) {
        if (!utils::contains(unchecked, id)) {
            Vertex *vertex = vertices.at(id);
            weightsToInf(*vertex, vertices, unchecked, events);
        }
    }
}

int Dijkstra::dijkstraAlgorithm(Vertex &startVertex, vertexMap vertices, edgeMap edges,
                                 std::vector<int>& checked, std::vector<int>& checkedEdges,
                                 std::vector<int>& unchecked, Events &events) {
    int currentVertex = startVertex.id;

    // Process incoming edges
    for (int id : startVertex.in.edgeId) {
        if (utils::contains(checkedEdges, id) || !utils::contains(checked, edges.at(id)->startId)) continue;

        checkedEdges.push_back(id);
        logEvent(events, CHECK_EDGE, x, id, x);
    }

    logEvent(events, SET_CURRENT_VERTEX, currentVertex, x, x);

    // Process neighboring vertices
    for (size_t i = 0; i < startVertex.out.vertexId.size(); ++i) {
        Vertex *vertex = vertices.at(startVertex.out.vertexId[i]);
        Edge *edge = edges.at(startVertex.out.edgeId[i]);

        bool isCheckedVertex = utils::contains(checked, vertex->id);

        checked.push_back(vertex->id);
        checkedEdges.push_back(edge->id);
        logEvent(events, CHECK_EDGE, x, edge->id, x);
        logEvent(events, CHECK_VERTEX, vertex->id, x, x);

        if (!isCheckedVertex) {
            qreal distToVertex = startVertex.weight + edge->weight;
            if (vertex->weight > distToVertex || vertex->weight == INF) {
                vertex->weight = distToVertex;
                logEvent(events, SET_WEIGHT, vertex->id, x, distToVertex);
            }

            checked.pop_back();
            checkedEdges.pop_back();
            logEvent(events, UNCHECK_VERTEX, vertex->id, x, x);
            logEvent(events, UNCHECK_EDGE, x, edge->id, x);
        }
    }

    checked.push_back(currentVertex);
    unchecked.erase(std::remove(unchecked.begin(), unchecked.end(), currentVertex), unchecked.end());
    logEvent(events, CHECK_VERTEX, currentVertex, x, x);

    // Select next vertex
    while (unchecked.size() != 0) {
        int nextId = getMinWeightVertex(unchecked, vertices);
        if (nextId == -1) break;

        Vertex* nextVertex = vertices.at(nextId);
        currentVertex = dijkstraAlgorithm(*nextVertex, vertices, edges, checked, checkedEdges, unchecked, events);
    }

    return currentVertex;
}

Events Dijkstra::run(Vertex &startVertex, vertexMap vertices, edgeMap edges) {
    Events events;
    std::vector<int> unchecked;
    std::vector<int> checkedEdges;
    std::vector<int> checkedVertices;

    weightsToInf(startVertex, vertices, unchecked, events);
    startVertex.weight = 0;

    logEvent(events, SET_START_VERTEX, startVertex.id, x, x);
    logEvent(events, SET_WEIGHT, startVertex.id, x, 0);

    int lastVertex = dijkstraAlgorithm(startVertex, vertices, edges, checkedVertices, checkedEdges, unchecked, events);

    logEvent(events, SET_END_VERTEX, lastVertex, x, x);

    return events;
}
