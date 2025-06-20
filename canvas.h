#ifndef CANVAS_H
#define CANVAS_H

#include "vertex.h"
#include "edge.h"
#include "Tools/tools.h"
#include "Tools/selecttool.h"
#include "Tools/pentool.h"
#include "dijkstra.h"

#include <vector>
#include <unordered_map>

#include <QMainWindow>

typedef std::unordered_map<int, Vertex*> vertexMap;
typedef std::unordered_map<int, Edge*> edgeMap;

class Canvas : public QMainWindow  {
    Q_OBJECT

public:
    Canvas(QWidget *parent = nullptr);

    Vertex* getClickedVertex(QPointF clickPos);
    Vertex* getVertex(int id) { return vertices.at(id); };
    QPointF getScreenCenter() { return screenCenter; };
    QPointF getTransformedPos(const QPointF& pos);
    QPointF getAbsoluteCenter();
    qreal getHalfScreenDiagonal() { return halfScreenDiagonal; };
    void createVertex(QPointF pos, int radius);
    void deselectAllVertices();
    void selectVertex(int id);

    const qreal EDGE_SELECTION_RANGE = 15;
    const int VERTEX_RADIUS = 25;
    QFont font = {"Latin Modern Math", 16};

    vertexMap vertices;
    edgeMap edges;
    std::vector<int> selectedEdges;

    std::vector<int> djCheckedEdges;
    std::vector<int> djCheckedVertices;
    vertexMap djEndAnimation;
    int djStartVertex = -1;
    int djEndVertex = -1;
    int djCurrentVertex = -1;

    qreal scaleFactor = 1.0;
    QPointF offset = {0, 0};

    qreal halfScreenDiagonal;
    QPointF screenCenter;

    Vertex* draggingVertex = nullptr;
    QPointF draggingOffset;

private:
    int getNumFromArray(std::vector<int> array);
    int getMinWeightVertex(std::vector<int> vertexIds);
    void resetInputState();
    void deselectFirstVertex();
    void linkVertices(int firstId, int secondId, qreal weight);
    void deleteEdge(int id);
    void deleteVertex(int id);

    void drawVertices(QPainter& painter);
    void drawEdges(QPainter& painter);
    void drawFakeEdges(QPainter& painter);
    void drawGrid(QPainter& painter, const QPointF& center);
    void drawTutorial(QPainter& painter);

    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void cancelDijkstra();
    void visualizeDijkstra(const vertexMap &graphVertices, const Events &events, int startIteretion);

    const QCursor PAN_CURSOR = Qt::ClosedHandCursor;
    QFont textFont = {"Latin Modern Math", 13};
    QStringList tutorialText = {
        "Use middle mouse button for navigation",
        "Place vertices using pen tool",
        "Select two vertices and start typing to enter the weight between",
        "Press \"Enter\" or \"E\" to confirm input",
        "Press space to start entering weight in another direction",
        "Press Delete to delete vertices or edges",
        "Select vertex and Run Dijkstra algorithm",
        "\"F\" - Run Dijkstra algorithm",
        "\"V\" - Select Tool",
        "\"B\" - Pen Tool",
        "\"A\" - Select all",
        "\"D\" - Deselect all"
    };

    const qreal ZOOM_OUT_LIMIT = 0.25;
    const qreal LINE_THICKNESS = 5;
    const qreal GRID_GAP = 16;
    const int gridLightnes = 150;
    const int GRID_DIVISON = 5;

    const int STEP_DELAY_MS = 400;
    const int START_DELAY_MS = 800;
    const int EDGE_STEP_DELAY_MS = STEP_DELAY_MS / 2;
    const int END_DELAY_MS = STEP_DELAY_MS / 4;
    const int FLICK_DELAY_MS = STEP_DELAY_MS / 2;

    int totalVertices = 0;
    int totalEdges = 0;

    std::vector<int> selectedVertices;

    QPoint lastMousePos;

    SelectTool *selectTool = new SelectTool(this);
    PenTool *penTool = new PenTool(this);
    Tools *currentTool = selectTool;

    bool isShiftPressed = false;
    Edge *fakeEdge = new Edge("", -1, 0, 0, 0, this);
    bool isFirstLink = true;
    std::vector<int> intPressed1;
    std::vector<int> intPressed2;
    int floatExponent1 = 0;
    int floatExponent2 = 0;

    bool isDijkstraRunning = false;
    int iteretion = 0;
};

#endif // CANVAS_H
