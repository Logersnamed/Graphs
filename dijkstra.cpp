#include "dijkstra.h"

#define INF -1
#define ANY -1

Dijkstra::Dijkstra() {

}

static bool contains(std::vector<int> vector, int value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

void Dijkstra::logEvent(EventName name, int vertexId, int edgeId, qreal weight) {
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

void Dijkstra::weightsToInf(Vertex& startVertex, vertexMap vertices, std::vector<int>& unchecked) {
    unchecked.push_back(startVertex.id);
    startVertex.weight = INF;
    logEvent(SET_WEIGHT, startVertex.id, ANY, INF);

    for (int id : startVertex.out.vertexId) {
        if (!contains(unchecked, id)) {
            Vertex *vertex = vertices.at(id);
            weightsToInf(*vertex, vertices, unchecked);
        }
    }
}

void Dijkstra::dijkstraAlgorithm(Vertex &startVertex, vertexMap vertices, edgeMap edges, std::vector<int>& checked) {
    dCurrVertex = startVertex.id;

    // Process incoming edges
    for (int id : startVertex.in.edgeId) {
        if (contains(dCheckedEdges, id) || !contains(checked, edges.at(id)->startId)) continue;

        dCheckedEdges.push_back(id);
        logEvent(CHECK_EDGE, ANY, id, ANY);
    }

    logEvent(SET_CURRENT_VERTEX, dCurrVertex, ANY, ANY);

    // Process neighboring vertices
    for (size_t i = 0; i < startVertex.out.vertexId.size(); ++i) {
        Vertex *vertex = vertices.at(startVertex.out.vertexId[i]);
        Edge *edge = edges.at(startVertex.out.edgeId[i]);

        bool isCheckedVertex = contains(checked, vertex->id);

        dCheckedVertices.push_back(vertex->id);
        dCheckedEdges.push_back(edge->id);
        logEvent(CHECK_EDGE, ANY, edge->id, ANY);
        logEvent(CHECK_VERTEX, vertex->id, ANY, ANY);


        if (!isCheckedVertex) {
            qreal distToVertex = startVertex.weight + edge->weight;
            if (vertex->weight > distToVertex || vertex->weight == INF) {
                vertex->weight = distToVertex;
                logEvent(SET_WEIGHT, vertex->id, ANY, distToVertex);
            }

            dCheckedVertices.pop_back();
            dCheckedEdges.pop_back();
            logEvent(UNCHECK_VERTEX, vertex->id, ANY, ANY);
            logEvent(UNCHECK_EDGE, ANY, edge->id, ANY);
        }

    }

    checked.push_back(dCurrVertex);
    dUnchecked.erase(std::remove(dUnchecked.begin(), dUnchecked.end(), dCurrVertex), dUnchecked.end());
    logEvent(CHECK_VERTEX, dCurrVertex, ANY, ANY);

    // Select next vertex
    while (dUnchecked.size() != 0) {
        int nextId = getMinWeightVertex(dUnchecked, vertices);
        if (nextId == -1) break;

        Vertex* nextVertex = vertices.at(nextId);
        dijkstraAlgorithm(*nextVertex, vertices, edges, checked);
    }
}

Events Dijkstra::run(Vertex &startVertex, vertexMap vertices, edgeMap edges) {
    weightsToInf(startVertex, vertices, dUnchecked);
    startVertex.weight = 0;

    logEvent(SET_START_VERTEX, startVertex.id, ANY, ANY);
    logEvent(SET_WEIGHT, startVertex.id, ANY, 0);

    dijkstraAlgorithm(startVertex, vertices, edges, dCheckedVertices);

    logEvent(SET_END_VERTEX, dCurrVertex, ANY, ANY);

    return events;
}
