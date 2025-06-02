#include "utils.h"
#include "canvas.h"
#include "dijkstra.h"

#include <unordered_map>
#include <algorithm>

#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QPainterPath>
#include <QEventLoop>
#include <QTimer>

Canvas::Canvas(QWidget *parent) : QMainWindow(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setFocus();
}

void delay(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

QPointF Canvas::getTransformedPos(const QPointF& pos) {
    QTransform transform;
    transform.translate(offset.x(), offset.y());
    transform.scale(scaleFactor, scaleFactor);
    return transform.inverted().map(pos);
}

int Canvas::getNumFromArray(std::vector<int> array) {
    int num = 0;
    int i = array.size();
    for (int digit : array) {
        num += digit * pow(10, i - 1);
        --i;
    }
    return num;
}

QPointF Canvas::getAbsoluteCenter() {
    QSize windowSize = this->size();
    return {windowSize.rwidth() / 2.0f, windowSize.rheight() / 2.0f};
}

Vertex* Canvas::getClickedVertex(QPointF clickPos) {
    Vertex* clickedVertex = nullptr;
    for (const auto& [id, vertex] : vertices) {
        bool isInRadius = QLineF(clickPos, vertex->pos).length() <= vertex->radius;
        if (!isInRadius) continue;

        clickedVertex = vertex;
    }
    return clickedVertex;
}

void getSubGraphVertices(const Vertex &startVertex, const vertexMap &vertices, vertexMap &subGraph) {
    subGraph.insert({startVertex.id, vertices.at(startVertex.id)});

    for (int id : startVertex.out.vertexId) {
        if (subGraph.find(id) == subGraph.end()) {
            Vertex *vertex = vertices.at(id);
            getSubGraphVertices(*vertex, vertices, subGraph);
        }
    }
}

void Canvas::resetInputState() {
    intPressed1.clear();
    intPressed2.clear();
    isFirstLink = true;
    floatExponent1 = 0;
    floatExponent2 = 0;
}

void Canvas::selectVertex(int id) {
    vertices.at(id)->isSelected = true;
    selectedVertices.push_back(id);
}

void Canvas::deselectFirstVertex() {
    vertices.at(selectedVertices[0])->isSelected = false;
    selectedVertices.erase(selectedVertices.begin());
}

void Canvas::deselectAllVertices() {
    for (int id : selectedVertices) {
        vertices.at(id)->isSelected = false;
    }
    selectedVertices.clear();
}

void Canvas::createVertex(QPointF pos, int radius) {
    QString name = QString::number(totalVertices);
    vertices.insert({totalVertices, new Vertex(name, totalVertices, radius, pos, this)});

    if (selectedVertices.size() > 2) {
        deselectAllVertices();
    }
    else if (selectedVertices.size() == 2) {
        deselectFirstVertex();
    }

    selectVertex(totalVertices);
    ++totalVertices;

    update();
}

void Canvas::linkVertices(int firstId, int secondId, qreal weight) {
    if (utils::contains(vertices.at(firstId)->out.vertexId, secondId) || firstId == secondId) return;

    vertices.at(secondId)->in.vertexId.push_back(firstId);
    vertices.at(secondId)->in.edgeId.push_back(totalEdges);
    vertices.at(firstId)->out.vertexId.push_back(secondId);
    vertices.at(firstId)->out.edgeId.push_back(totalEdges);

    edges.insert({totalEdges, new Edge(QString::number(weight), totalEdges, firstId, secondId, weight)});

    ++totalEdges;

    update();
}

void Canvas::drawVertices(QPainter& painter) {
    painter.setBrush(Qt::white);
    painter.setPen(Qt::black);

    for (const auto& [id, vertex] : vertices) {
        vertex->draw(this, painter);
    }
}

void Canvas::drawEdges(QPainter& painter) {
    painter.setBrush(Qt::black);

    for (const auto& [id, edge] : edges) {
        bool betweenSelected = vertices.at(edge->startId)->isSelected && vertices.at(edge->endId)->isSelected;
        bool isBoth = betweenSelected && intPressed1.size() > 0;
        edge->draw(this, painter, isBoth);
    }
}

void Canvas::drawFakeEdges(QPainter& painter) {
    if (intPressed1.size() <= 0) return;

    fakeEdge->startId = selectedVertices[isShiftPressed];
    fakeEdge->endId = selectedVertices[!isShiftPressed];
    if (!utils::contains(vertices.at(fakeEdge->startId)->out.vertexId, fakeEdge->endId)) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::green);
        painter.setOpacity(0.3);

        qreal weight = getNumFromArray(intPressed1) / (floatExponent1 ? floatExponent1 : 1.f);
        fakeEdge->displayText = QString::number(weight) + (floatExponent1 == 1 && isFirstLink ? "." : "");
        fakeEdge->draw(this, painter, !isFirstLink);
    }

    if (isFirstLink) return;

    fakeEdge->startId = selectedVertices[!isShiftPressed];
    fakeEdge->endId = selectedVertices[isShiftPressed];

    if (utils::contains(vertices.at(fakeEdge->startId)->out.vertexId, fakeEdge->endId)) return;

    if (intPressed2.size() <= 0) {
        fakeEdge->weight = -1;
        fakeEdge->displayText = "";
    }
    else {
        qreal weight = getNumFromArray(intPressed2)  / (floatExponent2 ? floatExponent2 : 1.f);
        fakeEdge->displayText = QString::number(weight) + (floatExponent2 == 1 ? "." : "");
    }

    fakeEdge->draw(this, painter, true);
}

void Canvas::drawGrid(QPainter& painter, const QPointF& center) {
    const qreal xCenterOffset = center.x() / scaleFactor;
    const qreal yCenterOffset = center.y() / scaleFactor;

    const qreal leftBorder   = screenCenter.x() - xCenterOffset;
    const qreal rightBorder  = screenCenter.x() + xCenterOffset;
    const qreal topBorder    = screenCenter.y() - yCenterOffset;
    const qreal bottomBorder = screenCenter.y() + yCenterOffset;

    const qreal gap = scaleFactor > 0.8 ? GRID_GAP : GRID_GAP * GRID_DIVISON;
    const int actualDivision = scaleFactor > 0.8 ? GRID_DIVISON : 1;

    const int left   = utils::absCeil((leftBorder - LINE_THICKNESS) / gap);
    const int right  = utils::absCeil((rightBorder + LINE_THICKNESS) / gap);
    const int top    = utils::absCeil((topBorder - LINE_THICKNESS) / gap);
    const int bottom = utils::absCeil((bottomBorder + LINE_THICKNESS) / gap);

    QColor gridColor = QColor(gridLightnes, gridLightnes, gridLightnes, 255);
    for (int i = left; i <= right; ++i) {
        painter.setPen(QPen(gridColor, 0.1f));

        if (i == 0) painter.setPen(QPen(QColor(gridLightnes - 50, gridLightnes - 50, gridLightnes - 50, 255), 1));
        else if (i % actualDivision * actualDivision == 0) painter.setPen(QPen(gridColor, 0.5f));
        else if (i % actualDivision == 0) painter.setPen(QPen(gridColor, 0.3f));

        painter.drawLine(QLineF({gap * i, bottomBorder},
                                {gap * i, topBorder}));
    }

    for (int i = top; i <= bottom; ++i) {
        painter.setPen(QPen(gridColor, 0.1f));

        if (i == 0) painter.setPen(QPen(QColor(gridLightnes - 50, gridLightnes - 50, gridLightnes - 50, 255), 1));
        else if (i % actualDivision * actualDivision == 0) painter.setPen(QPen(gridColor, 0.5f));
        else if (i % actualDivision == 0) painter.setPen(QPen(gridColor, 0.3f));

        painter.drawLine(QLineF({rightBorder , gap * i},
                                {leftBorder , gap * i}));
    }

    painter.setPen(QPen(Qt::black, 1));
}

void Canvas::drawTutorial(QPainter& painter) {
    const int startY = 5;
    const int lineHeight = 18;
    const int textPaddingX = 6;
    const int textPaddingY = 18;
    const int rectOffsetY = 14;
    const int textOffsetX = 13;

    int y = startY;

    for (const QString& line : tutorialText) {
        y += lineHeight;

        int textWidth = painter.fontMetrics().horizontalAdvance(line);
        QRect textRect(10, y - rectOffsetY, textWidth + textPaddingX, lineHeight);
        painter.fillRect(textRect, Qt::white);

        painter.drawText(textOffsetX, y, line);
    }
}

void Canvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(textFont);
    painter.translate(offset);
    painter.scale(scaleFactor, scaleFactor);

    QPointF center = getAbsoluteCenter();
    screenCenter = (center - offset) / scaleFactor;
    halfScreenDiagonal = qSqrt(QPointF::dotProduct(center, center)) / scaleFactor;

    drawGrid(painter, center);
    drawTutorial(painter);

    painter.setFont(font);

    drawEdges(painter);
    drawFakeEdges(painter);
    drawVertices(painter);
}

void Canvas::deleteEdge(int id) {
    Edge *edge = edges.at(id);
    Vertex *start = vertices.at(edge->startId);
    Vertex *end = vertices.at(edge->endId);

    utils::removeFromBothByFirst(start->out.edgeId, start->out.vertexId, id);
    utils::removeFromBothByFirst(end->in.edgeId, end->in.vertexId, id);

    delete edge;
    edges.erase(id);
}

void Canvas::deleteVertex(int id) {
    Vertex* vertex = vertices.at(id);
    std::vector<int> inEdges = vertex->in.edgeId;
    std::vector<int> outEdges = vertex->out.edgeId;

    for (int edgeId : inEdges) {
        deleteEdge(edgeId);
    }
    for (int edgeId : outEdges) {
        deleteEdge(edgeId);
    }

    delete vertex;
    vertices.erase(id);
}

void Canvas::cancelDijkstra() {
    for (const auto& [id, vertex] : vertices) {
        vertex->weight = -2;
    }

    ++iteretion;
    isDijkstraRunning = false;
    djCheckedVertices.clear();
    djCheckedEdges.clear();
    djEndAnimation.clear();
    djStartVertex = -1;
    djEndVertex = -1;
    djCurrentVertex = -1;

    update();
}

void Canvas::visualizeDijkstra(const vertexMap &graphVertices, const Events &events, int startIteretion) {
    for (Event event : events) {
        if (startIteretion != iteretion) break;

        if (event.name == SET_START_VERTEX) {
            djStartVertex = event.vertexId;
            delay(START_DELAY_MS);
        }
        else if (event.name == SET_CURRENT_VERTEX) {
            djCurrentVertex = event.vertexId;
            delay(STEP_DELAY_MS);
            update();
            delay(STEP_DELAY_MS);
        }
        else if (event.name == SET_END_VERTEX) {
            djEndVertex = event.vertexId;
        }
        else if (event.name == CHECK_VERTEX) {
            djCheckedVertices.push_back(event.vertexId);
            delay(STEP_DELAY_MS);
        }
        else if (event.name == CHECK_EDGE) {
            djCheckedEdges.push_back(event.edgeId);
        }
        else if (event.name == UNCHECK_VERTEX) {
            djCheckedVertices.erase(std::remove(djCheckedVertices.begin(), djCheckedVertices.end(), event.vertexId), djCheckedVertices.end());
        }
        else if (event.name == UNCHECK_EDGE) {
            djCheckedEdges.erase(std::remove(djCheckedEdges.begin(), djCheckedEdges.end(), event.edgeId), djCheckedEdges.end());
            delay(EDGE_STEP_DELAY_MS);
        }
        else if (event.name == SET_WEIGHT) {
            vertices.at(event.vertexId)->weight = event.weight;
        }

        update();
    }

    for (const auto& vertex : graphVertices) {
        if (startIteretion != iteretion) break;

        delay(END_DELAY_MS);
        djEndAnimation.insert(vertex);
        update();
    }

    for (int i = 0; i < 3; ++i) {
        if (startIteretion != iteretion) break;

        delay(FLICK_DELAY_MS);
        djEndAnimation = graphVertices;
        update();

        delay(FLICK_DELAY_MS);
        djEndAnimation.clear();
        update();
    }
}

void Canvas::wheelEvent(QWheelEvent *event) {
    QPointF cursorPos = event->position();
    QPointF scenePos = (cursorPos - offset) / scaleFactor;

    qreal factor = (event->angleDelta().y() > 0) ? 1.2 : 0.8;
    if (scaleFactor > ZOOM_OUT_LIMIT || factor > 1) {
        scaleFactor *= factor;
    }

    offset = cursorPos - scenePos * scaleFactor;

    update();
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        resetInputState();

        currentTool->onLeftClick(event);
    }

    if (event->button() == Qt::MiddleButton) {
        lastMousePos = event->pos();
        this->setCursor(PAN_CURSOR);
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (draggingVertex && (event->buttons() & Qt::LeftButton)) {
        QPointF transformedPos = getTransformedPos(event->pos());
        QPointF mainVertPos = draggingVertex->pos;

        for (int id : selectedVertices) {
            QPointF vertOffset = mainVertPos - vertices.at(id)->pos;
            vertices.at(id)->pos = transformedPos + draggingOffset - vertOffset;
        }

        update();
    }

    if (event->buttons() & Qt::MiddleButton) {
        QPoint delta = event->pos() - lastMousePos;
        lastMousePos = event->pos();
        offset += delta;
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    draggingVertex = nullptr;
    draggingOffset = {0, 0};
    this->setCursor(currentTool->getCursor());
}

void Canvas::keyPressEvent(QKeyEvent *event) {
    int key = event->key();

    if (key == Qt::Key_Return || key == Qt::Key_E) {
        if (!intPressed1.size()) return;
        if (selectedVertices.size() != 2) return;

        linkVertices(selectedVertices[isShiftPressed], selectedVertices[!isShiftPressed], getNumFromArray(intPressed1) / (floatExponent1 ? floatExponent1 : 1.f));
        if (intPressed2.size()) linkVertices(selectedVertices[!isShiftPressed], selectedVertices[isShiftPressed], getNumFromArray(intPressed2) / (floatExponent2 ? floatExponent2 : 1.f));

        selectedEdges.clear();
        deselectFirstVertex();

        resetInputState();
        update();
        return;
    }

    if (key >= '0' && key <= '9') {
        if (selectedVertices.size() != 2) return;
        if (utils::contains(vertices.at(selectedVertices[0])->out.vertexId, selectedVertices[1])) return;
        if (isDijkstraRunning) return;

        if (isFirstLink) {
            if (intPressed1.size() == 0 && key == 0 ) return;
            if (intPressed1.size() == 6) return;

            intPressed1.push_back(key - '0');
            if (floatExponent1) floatExponent1 *= 10;
        }
        else {
            if (utils::contains(vertices.at(selectedVertices[1])->out.vertexId, selectedVertices[0])) return;
            if (intPressed2.size() == 0 && key == 0 ) return;
            if (intPressed2.size() == 6) return;

            intPressed2.push_back(key - '0');
            if (floatExponent2) floatExponent2 *= 10;
        }

        update();
        return;
    }

    if (key == '.' || key == ',') {
        if (!intPressed1.size()) return;
        if (isFirstLink) {
            if (!floatExponent1) floatExponent1 = 1;
        }
        else {
            if (!floatExponent2) floatExponent2 = 1;
        }

        update();
        return;
    }

    if (key == Qt::Key_Space) {
        if (!intPressed1.size()) return;
        isFirstLink = false;

        update();
        return;
    }

    if (key == Qt::Key_Backspace) {
        if (!intPressed1.size()) return;

        if (isFirstLink) {
            intPressed1.pop_back();
            if (floatExponent1) floatExponent1 /= 10;
        }
        else {
            if (!intPressed2.size()) {
                floatExponent2 = 0;
                isFirstLink = true;
                update();
                return;
            }
            intPressed2.pop_back();
            if (!intPressed2.size()) {
                floatExponent2 = 0;
            }
            else if (floatExponent2) {
                floatExponent2 /= 10;
            }
        }

        update();
        return;
    }

    if (key == Qt::Key_V) {
        currentTool = selectTool;
        this->setCursor(currentTool->getCursor());
        return;
    }

    if (key == Qt::Key_B) {
        currentTool = penTool;
        this->setCursor(currentTool->getCursor());
        return;
    }

    if (key == Qt::Key_F) {
        cancelDijkstra();

        if (selectedVertices.size() != 1) return;

        selectedEdges.clear();
        deselectAllVertices();
        isDijkstraRunning = true;
        int startIteretion = iteretion;

        Vertex *startVertex = vertices.at(selectedVertices[0]);
        vertexMap subGraphVertices;
        getSubGraphVertices(*startVertex, vertices, subGraphVertices);
        Events events = Dijkstra::run(*startVertex, subGraphVertices, edges);

        visualizeDijkstra(subGraphVertices, events, startIteretion);

        isDijkstraRunning = false;
        resetInputState();
        return;
    }

    if (key == Qt::Key_D) {
        deselectAllVertices();

        resetInputState();
        update();
        return;
    }

    if (key == Qt::Key_A) {
        deselectAllVertices();

        for (auto& [id, vertex] : vertices) {
            selectVertex(id);
        }

        update();
        return;
    }

    if (key == Qt::Key_Z) {
        if (isDijkstraRunning || selectedVertices.size() < 2) return;

        for (int i = 0; i < selectedVertices.size(); ++i) {
            for (int j = i + 1; j < selectedVertices.size(); ++j) {
                linkVertices(selectedVertices[i], selectedVertices[j], 1);
            }
        }

        resetInputState();
        update();
        return;
    }

    if (key == Qt::Key_Delete) {
        if (selectedVertices.size() <= 0 && selectedEdges.size() <= 0 || isDijkstraRunning) return;

        for (int id : selectedEdges) {
            deleteEdge(id);
        }
        selectedEdges.clear();

        for (int id : selectedVertices) {
            deleteVertex(id);
        }
        selectedVertices.clear();

        resetInputState();
        update();
        return;
    }

    if (key == Qt::Key_Shift) {
        isShiftPressed = true;
        update();
        return;
    }
}

void Canvas::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        isShiftPressed = false;
        update();
    }
}
