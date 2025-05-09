#ifndef CANVAS_H
#define CANVAS_H

#include "vertex.h"
#include "edge.h"
#include "Tools/tools.h"
#include "Tools/selecttool.h"
#include "Tools/pentool.h"
#include "mainwindow.h"

#include <vector>
#include <unordered_map>

#include <QWidget>

typedef std::unordered_map<int, Vertex*> vertexMap;
typedef std::unordered_map<int, Edge*> edgeMap;

class Canvas : public QWidget {
    Q_OBJECT

    QPoint lastMousePos;
    QPointF offset = {0, 0};
    qreal scaleFactor = 1.0;

    QPointF screenCenter;
    qreal halfScreenDiagonal;

    SelectTool *selectTool = new SelectTool();
    PenTool *penTool = new PenTool(this);
    Tools *currentTool = selectTool;

public:
    int totalVertices = 0;
    int totalEdges = 0;
    vertexMap vertices;
    edgeMap edges;

    bool isFirstLink = true;
    std::vector<int> intPressed1;
    std::vector<int> intPressed2;

    std::vector<int> selectedVertices;
    std::vector<int> selectedEdges;

    Edge *fakeEdge = new Edge(-1, 0, 0, 0, this);

    Vertex* draggingVertex = nullptr;
    QPointF draggingOffset;

    const qreal ZOOM_OUT_LIMIT = 0.25;
    const int VERTEX_RADIUS = 25;
    const QPointF WEIGHT_TEXT_OFFSET = {0, - VERTEX_RADIUS - 15.0};
    const qreal EDGE_TEXT_SHIFT = 15;
    const qreal EDGE_BOTH_SHIFT = 12;
    const qreal EDGE_SELECTION_RANGE = 15;
    const qreal ARROW_LENGTH = 13;
    const qreal ARROW_ANGLE = 13;
    const qreal LINE_THICKNESS = 5;
    const qreal GRID_GAP = 16;
    const int GRID_DIVISON = 5;
    const int STEP_DELAY_MS = 300;
    // const QFont font = {"Times New Roman", 16};
    // const QFont font = {"Cambria", 16};
    QFont font = {"Latin Modern Math", 16};
    const QCursor PAN_CURSOR = Qt::ClosedHandCursor;

    MainWindow *mainWindow;

    Canvas(MainWindow *parentWindow, QWidget *parent = nullptr);

    void createVertex(QPointF pos, int radius);
    QPointF getTransformedPos(const QPointF& pos);

    std::vector<QPointF> debugPoints;
    std::vector<QLineF> debugLines;

    void deleteEdge(int id);
    void deleteVertex(int id);

private:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    QPointF getClosestPointToEdge(QLineF edgeLine, QString text, QFontMetrics fontMetrics);
    void drawEdge(QPainter& painter, int id, QPointF startPos, QPointF endPos, int value, int startId, int endId, bool isForceBoth);
    void drawEdge(QPainter& painter, Edge *edge, bool isForceBoth);
    void drawArrow(QPainter& painter, QLineF invertedEdgeLine, qreal vertexRadius);
    void drawFakeEdges(QPainter& painter);
    void drawGrid(QPainter& painter, const QPointF& center);

    void drawDebug(QPainter& painter);

    bool isVertexSelected(int vertexId);
    bool isEdgeSelected(const Edge& edge);
    bool isInMap(intMap map, int key);
    QPointF getAbsoluteCenter();
    void drawVertices(QPainter& painter);
    void drawEdges(QPainter& painter);
    int getNumFromArray(std::vector<int> array);
    void linkVertices(int firstId, int secondId, int value);

    // void weightsToInf(Vertex& startVertex, std::vector<int> checked);
    // std::vector<int> sortByEdgeWeights(std::vector<int> vertexIds, std::vector<int> edgeIds);
    // void dijkstraAlgorithm(Vertex &startVertex, std::vector<int>& checked);
    // void Dijkstra(Vertex& startVertex);

    void delay(int milliseconds);

    void Dijkstra(Vertex &startVertex);
    void weightsToInf(Vertex& startVertex, std::vector<int>& checked); // Pass by reference now
    void dijkstraAlgorithm(Vertex &startVertex, std::vector<int>& checked);
    // std::vector<int> sortByEdgeWeights(std::vector<int> vertexIds, std::vector<int> edgeIds);
    std::vector<std::pair<int, int>> sortByEdgeWeights(std::vector<int> vertexIds, std::vector<int> edgeIds);
    std::vector<std::pair<int, int>> sortByWeights(std::vector<int> vertexIds);

    bool isDijkstraRunning = false;
    std::vector<int> dUnchecked;
    std::vector<int> dCheckedEdges;
    std::vector<int> dCheckedVertices;
    int dFirst = -10;
    int dEnd = -10;
    int dCurrVertex = -10;

    const QColor dFirstColor = QColor(255, 228, 212);
    const QColor dChekcedColor = QColor(255, 180, 162);
    const QColor dCurrColor = QColor(229, 152, 155);
    const QColor dEndColor = QColor(181, 131, 141);
    const QColor dWeightColor = QColor(109, 104, 117);
};

#endif // CANVAS_H
