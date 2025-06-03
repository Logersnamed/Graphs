#include "utils.h"
#include "dijkstra.h"

void Dijkstra::logEvent(Events &events, EventName name, int vertexId, int edgeId, qreal weight) {
    events.emplace_back(Event{name, vertexId, edgeId, weight});
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

    return (minWeight == INF || minWeight == UNDEFINED) ? INF : minId;
}

void Dijkstra::weightsToInf(Vertex& startVertex, vertexMap vertices, std::vector<int>& unchecked, Events &events) {
    unchecked.push_back(startVertex.id);
    startVertex.weight = INF;
    logEvent(events, SET_WEIGHT, startVertex.id, UNDEFINED, INF);

    for (int id : startVertex.out.vertexId) {
        if (!utils::contains(unchecked, id)) {
            Vertex *vertex = vertices.at(id);
            weightsToInf(*vertex, vertices, unchecked, events);
        }
    }
}

int Dijkstra::dijkstraAlgorithm(Vertex &startVertex, vertexMap vertices, edgeMap edges,
                                std::vector<int>& checkedEdges, std::vector<int>& unchecked, Events &events) {
    int currentVertex = startVertex.id;

    // Process incoming edges
    for (int id : startVertex.in.edgeId) {
        if (utils::contains(checkedEdges, id) || utils::contains(unchecked, edges.at(id)->startId)) continue;

        checkedEdges.push_back(id);
        logEvent(events, CHECK_EDGE, UNDEFINED, id, UNDEFINED);
    }

    logEvent(events, SET_CURRENT_VERTEX, currentVertex, UNDEFINED, UNDEFINED);

    // Process neighboring vertices
    for (size_t i = 0; i < startVertex.out.vertexId.size(); ++i) {
        Vertex *vertex = vertices.at(startVertex.out.vertexId[i]);
        Edge *edge = edges.at(startVertex.out.edgeId[i]);

        if (!utils::contains(checkedEdges, edge->id)) {
            checkedEdges.push_back(edge->id);
            logEvent(events, CHECK_EDGE, UNDEFINED, edge->id, UNDEFINED);
        }

        if (utils::contains(unchecked, vertex->id)) {
            logEvent(events, CHECK_VERTEX, vertex->id, UNDEFINED, UNDEFINED);

            qreal distToVertex = startVertex.weight + edge->weight;
            if (vertex->weight > distToVertex || vertex->weight == INF) {
                vertex->weight = distToVertex;
                logEvent(events, SET_WEIGHT, vertex->id, UNDEFINED, distToVertex);
            }

            checkedEdges.pop_back();
            logEvent(events, UNCHECK_VERTEX, vertex->id, UNDEFINED, UNDEFINED);
            logEvent(events, UNCHECK_EDGE, UNDEFINED, edge->id, UNDEFINED);
        }
    }

    unchecked.erase(std::remove(unchecked.begin(), unchecked.end(), currentVertex), unchecked.end());
    logEvent(events, CHECK_VERTEX, currentVertex, UNDEFINED, UNDEFINED);

    // Select next vertex
    while (unchecked.size() != 0) {
        int nextId = getMinWeightVertex(unchecked, vertices);
        if (nextId == INF) break;

        Vertex* nextVertex = vertices.at(nextId);
        currentVertex = dijkstraAlgorithm(*nextVertex, vertices, edges, checkedEdges, unchecked, events);
    }

    return currentVertex;
}

Events Dijkstra::run(Vertex &startVertex, vertexMap vertices, edgeMap edges) {
    Events events;
    std::vector<int> unchecked, checkedEdges;

    weightsToInf(startVertex, vertices, unchecked, events);
    startVertex.weight = 0;

    logEvent(events, SET_START_VERTEX, startVertex.id, UNDEFINED, UNDEFINED);
    logEvent(events, SET_WEIGHT, startVertex.id, UNDEFINED, 0);

    int lastVertex = dijkstraAlgorithm(startVertex, vertices, edges, checkedEdges, unchecked, events);

    logEvent(events, SET_END_VERTEX, lastVertex, UNDEFINED, UNDEFINED);

    return events;
}
