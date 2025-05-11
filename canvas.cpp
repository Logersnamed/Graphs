#include "canvas.h"
#include "vertex.h"
#include "edge.h"
#include "mainwindow.h"

#include <unordered_map>
#include <algorithm>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QPainterPath>
#include <QEventLoop>
#include <QTimer>

#define INF -1

typedef std::unordered_map<int, Vertex*> vertexMap;
typedef std::unordered_map<int, Edge*> edgeMap;
typedef std::unordered_map<int, int> intMap;

Canvas::Canvas(MainWindow *parentWindow, QWidget *parent) : QWidget(parent), mainWindow(parentWindow) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setFocus();

    for (int i = 0; i < 300; ++i) createVertex({0, i * 10.f}, VERTEX_RADIUS);
}

// QPointF Canvas::getVertexPos(int id) {
//     return vertices.at(id)->pos;
// }

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

    // qDebug() << "createVertex(" << pos << "," << radius << ");";
}

bool contains(std::vector<int> vector, int value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

void Canvas::linkVertices(int firstId, int secondId, qreal weight) {
    if (contains(vertices.at(firstId)->out.vertexId, secondId)) {
        qDebug() << "Already connected";
        return;
    }

    vertices.at(secondId)->in.vertexId.push_back(firstId);
    vertices.at(secondId)->in.edgeId.push_back(totalEdges);
    vertices.at(firstId)->out.vertexId.push_back(secondId);
    vertices.at(firstId)->out.edgeId.push_back(totalEdges);

    edges.insert({totalEdges, new Edge(totalEdges, firstId, secondId, weight)});

    ++totalEdges;

    update();

    // qDebug() << "linkVertices(" << firstId << "," << secondId << "," << weight << ");";
}

bool Canvas::isVertexSelected(int vertexId) {
    for (int selectedIds : selectedVertices) {
        if (vertexId == selectedIds) return true;
    }
    return false;
}

QPointF getTextCenterAlign(QFontMetrics fm, QString text) {
    int width = fm.horizontalAdvance(text);
    int height = fm.ascent() - fm.descent();

    return {-width / 2.0f, height / 2.0f};
}

QPointF Canvas::getAbsoluteCenter() {
    QSize mainWindowSize = mainWindow->size();
    return {mainWindowSize.rwidth() / 2.0f, mainWindowSize.rheight() / 2.0f};
}

void Canvas::drawVertices(QPainter& painter) {
    painter.setBrush(Qt::white);
    painter.setPen(Qt::black);

    for (const auto& [id, vertex] : vertices) {
        vertex->draw(this, painter);
    }
}

QPointF newVector(const QLineF& direction, const qreal length) {
    qreal directionLength = direction.length();
    qreal sin = direction.dy() / directionLength;
    qreal cos = direction.dx() / directionLength;

    return {length * cos, length * sin};
}

QPointF newVector(const QLineF& direction, const QPointF shift) {
    qreal directionLength = direction.length();
    qreal sin = direction.dy() / directionLength;
    qreal cos = direction.dx() / directionLength;

    return {shift.x() * cos, shift.y() * sin};
}

QPointF closestPoint(QLineF line, QLineF normal, QPointF origin) {
    QPointF intersectionPoint;
    line.intersects(normal, &intersectionPoint);

    QPointF closestPoint = intersectionPoint;

    qreal distToStart = QLineF{intersectionPoint, line.p1()}.length();
    qreal distToEnd = QLineF{intersectionPoint, line.p2()}.length();
    qreal edgeLineLength = line.length();

    if (distToStart > edgeLineLength || distToEnd > edgeLineLength) {
        if (QLineF{origin, line.p1()}.length() > QLineF{origin, line.p2()}.length()) {
            return line.p2();
        }
        else {
            return line.p1();
        }
    }

    return closestPoint;
}

QPointF closestPoint(QString text, QPointF textPos, QPointF textCenterOffset, QPointF origin) {
    QLineF inTextLine{textPos - textCenterOffset, origin};
    qreal textRadius = qSqrt(QPointF::dotProduct(textCenterOffset, textCenterOffset));
    if (inTextLine.length() <= textRadius) return origin;
    inTextLine.setLength(textRadius);
    return inTextLine.p2();
}

void Canvas::drawArrow(QPainter& painter, QLineF invertedEdgeLine, qreal vertexRadius) {
    QLineF line = invertedEdgeLine;
    qreal distToCircle = sqrt(vertexRadius * vertexRadius - EDGE_BOTH_SHIFT * EDGE_BOTH_SHIFT / 4);
    line.setLength(distToCircle);

    QLineF wing1{line.p2(), invertedEdgeLine.p2()};
    QLineF wing2{line.p2(), invertedEdgeLine.p2()};

    wing1.setLength(ARROW_LENGTH);
    wing2.setLength(ARROW_LENGTH);

    wing1.setAngle(line.angle() + ARROW_ANGLE);
    wing2.setAngle(line.angle() - ARROW_ANGLE);

    painter.setOpacity(1);

    QPainterPath path;
    path.moveTo(wing1.p1());
    path.lineTo(wing1.p2());
    path.lineTo(wing2.p2());
    painter.drawPath(path);
}

QLineF shiftLine(QLineF line, QLineF direction, qreal shiftValue) {
    QPointF shift = newVector(direction, shiftValue);
    return QLineF{line.p1() + shift, line.p2() + shift};
}

void Canvas::drawEdge(QPainter& painter, Edge *edge, QString text, bool isForceBoth) {
    Vertex* start = vertices.at(edge->startId);
    Vertex* end = vertices.at(edge->endId);
    QLineF edgeLine = {start->pos, end->pos};
    QLineF normal(screenCenter, {0, 0});
    normal.setAngle(edgeLine.angle() + 90);

    if (contains(vertices.at(edge->startId)->in.vertexId, edge->endId) || isForceBoth) {
        edgeLine = shiftLine(edgeLine, normal, EDGE_BOTH_SHIFT / 2);
    }

    QPointF textCenterOffset = getTextCenterAlign(painter.fontMetrics(), text);
    QPointF shift = newVector(normal, {EDGE_TEXT_SHIFT - textCenterOffset.x(), EDGE_TEXT_SHIFT + textCenterOffset.y()});
    QPointF textPos = edgeLine.center() + textCenterOffset + shift;

    qreal closestDist = std::min(QLineF{screenCenter, closestPoint(edgeLine, normal, screenCenter)}.length(),
                                 QLineF{screenCenter, closestPoint(text, textPos, textCenterOffset, screenCenter)}.length());

    if (closestDist - LINE_THICKNESS > halfScreenDiagonal) return;

    bool isSelected = start->isSelected && end->isSelected || contains(selectedEdges, edge->id);
    if (isSelected) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::green);
    }
    else if (contains(dCheckedEdges, edge->id)) {
        painter.setBrush(dChekcedColor);
        painter.setPen(dChekcedColor);
    }

    painter.drawLine(edgeLine);
    drawArrow(painter, {edgeLine.p2(), edgeLine.p1()}, vertices.at(edge->endId)->radius);

    if (isSelected) {
        painter.setPen(Qt::darkGreen);
    }
    else {
        painter.setPen(Qt::black);
    }

    painter.drawText(textPos, text);
    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
}

void Canvas::drawEdges(QPainter& painter) {
    painter.setBrush(Qt::black);

    for (const auto& [id, edge] : edges) {
        bool betweenSelected = isVertexSelected(edge->startId) && isVertexSelected(edge->endId);
        bool isBoth = betweenSelected && intPressed1.size() > 0;
        drawEdge(painter, edge, QString::number(edge->weight), isBoth);
    }
}

int absCeil(qreal value) {
    return value >= 0 ? ceil(value) : floor(value);
}

void Canvas::drawGrid(QPainter& painter, const QPointF& center) {
    // For now it doens't really care 'bout canvasCenter
    QPointF canvasCenter{0, 0};
    const qreal xCenterOffset = center.x() / scaleFactor;
    const qreal yCenterOffset = center.y() / scaleFactor;

    const qreal leftBorder   = screenCenter.x() - xCenterOffset;
    const qreal rightBorder  = screenCenter.x() + xCenterOffset;
    const qreal topBorder    = screenCenter.y() - yCenterOffset;
    const qreal bottomBorder = screenCenter.y() + yCenterOffset;

    const qreal gap = scaleFactor > 0.8 ? GRID_GAP : GRID_GAP * GRID_DIVISON;
    const int actualDivision = scaleFactor > 0.8 ? GRID_DIVISON : 1;

    const int left   = absCeil((leftBorder - canvasCenter.x() - LINE_THICKNESS) / gap);
    const int right  = absCeil((rightBorder - canvasCenter.x() + LINE_THICKNESS) / gap);
    const int top    = absCeil((topBorder - canvasCenter.y() - LINE_THICKNESS) / gap);
    const int bottom = absCeil((bottomBorder - canvasCenter.y() + LINE_THICKNESS) / gap);

    int lightness = 150;
    QColor gridColor = QColor(lightness, lightness, lightness, 255);

    for (int i = left; i <= right; ++i) {
        painter.setPen(QPen(gridColor, 0.1f));

        if (i == 0) painter.setPen(QPen(QColor(lightness - 50, lightness - 50, lightness - 50, 255), 1));
        else if (i % actualDivision * actualDivision == 0) painter.setPen(QPen(gridColor, 0.5f));
        else if (i % actualDivision == 0) painter.setPen(QPen(gridColor, 0.3f));

        painter.drawLine(QLineF({gap * i, bottomBorder},
                                {gap * i, topBorder}));
    }

    for (int i = top; i <= bottom; ++i) {
        painter.setPen(QPen(gridColor, 0.1f));

        if (i == 0) painter.setPen(QPen(QColor(lightness - 50, lightness - 50, lightness - 50, 255), 1));
        else if (i % actualDivision * actualDivision == 0) painter.setPen(QPen(gridColor, 0.5f));
        else if (i % actualDivision == 0) painter.setPen(QPen(gridColor, 0.3f));

        painter.drawLine(QLineF({rightBorder , gap * i},
                                {leftBorder , gap * i}));
    }

    painter.setPen(QPen(Qt::black, 1));
}

void Canvas::drawFakeEdges(QPainter& painter) {
    if (intPressed1.size() <= 0) return;

    painter.setBrush(Qt::green);
    painter.setPen(Qt::green);
    painter.setOpacity(0.3);

    fakeEdge->startId = selectedVertices[0];
    fakeEdge->endId = selectedVertices[1];
    fakeEdge->weight = getNumFromArray(intPressed1) / (floatExponent1 ? floatExponent1 : 1.f);
    drawEdge(painter, fakeEdge, QString::number(fakeEdge->weight) + (floatExponent1 == 1 && isFirstLink ? "." : ""), !isFirstLink);

    if (isFirstLink) return;

    fakeEdge->startId = selectedVertices[1];
    fakeEdge->endId = selectedVertices[0];

    if (intPressed2.size() <= 0) {
        fakeEdge->weight = -1;
        drawEdge(painter, fakeEdge, "", true);
    }
    else {
        fakeEdge->weight = getNumFromArray(intPressed2)  / (floatExponent2 ? floatExponent2 : 1.f);
        drawEdge(painter, fakeEdge, QString::number(fakeEdge->weight) + (floatExponent2 == 1 ? "." : ""), true);
    }
}

void Canvas::drawTutorial(QPainter& painter) {
    int h = 5;
    for (const QString& line : tutorialText) {
        h += 18;

        QRect textRect(10, h - 14, painter.fontMetrics().horizontalAdvance(line) + 6, 18);
        painter.fillRect(textRect, Qt::white);

        painter.drawText(13, h, line);
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

Vertex* Canvas::getClickedVertex(QPointF clickPos) {
    Vertex* clickedVertex = nullptr;
    for (const auto& [id, vertex] : vertices) {
        bool isInRadius = QLineF(clickPos, vertex->pos).length() <= vertex->radius;
        if (!isInRadius) continue;

        clickedVertex = vertex;
    }
    return clickedVertex;
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    QPointF clickPos = getTransformedPos(event->pos());

    if (event->button() == Qt::LeftButton) {
        resetInputState();

        if (currentTool == selectTool) {
            if (event->modifiers() != Qt::ShiftModifier) {
                deselectAllVertices();
                selectedEdges.clear();
            }

            Vertex* clickedVertex = getClickedVertex(clickPos);
            if (clickedVertex) {
                if (!clickedVertex->isSelected) {
                    selectVertex(clickedVertex->id);
                }

                draggingVertex = clickedVertex;
                draggingOffset = clickedVertex->pos - clickPos;

                update();
                return;
            }

            // Getting clicked edge
            QPointF center = getAbsoluteCenter();
            screenCenter = (center - offset) / scaleFactor;
            halfScreenDiagonal = qSqrt(QPointF::dotProduct(center, center)) / scaleFactor;

            qreal closest = - 1;
            int closestId = -1;
            for (const auto& [id, edge] : edges) {
                QLineF edgeLine = {vertices.at(edge->startId)->pos, vertices.at(edge->endId)->pos};
                QLineF normal(clickPos, {0, 0});
                normal.setAngle(edgeLine.angle() + 90);

                if (contains(vertices.at(edge->startId)->in.vertexId, edge->endId)) {
                    edgeLine = shiftLine(edgeLine, normal, EDGE_BOTH_SHIFT / 2);
                }

                QFontMetrics fm(font);
                QString text = QString::number(edge->weight);
                QPointF textCenterOffset = getTextCenterAlign(fm, text);
                QPointF shift = newVector(normal, {EDGE_TEXT_SHIFT - textCenterOffset.x(), EDGE_TEXT_SHIFT + textCenterOffset.y()});
                QPointF textPos = edgeLine.center() + textCenterOffset + shift;

                qreal closestDist = std::min(QLineF{clickPos, closestPoint(edgeLine, normal, clickPos)}.length(),
                                             QLineF{clickPos, closestPoint(text, textPos, textCenterOffset, clickPos)}.length());

                if (closestDist > EDGE_SELECTION_RANGE) continue;

                if (closest == -1) closest = closestDist;
                if (closestId == -1) closestId = id;
                if (closestDist < closest) {
                    closest = closestDist;
                    closestId = id;
                }
            }

            if (closestId != -1) selectedEdges.push_back(closestId);

            update();

            return;
        }

        currentTool->onLeftClick(event);
    }

    if (event->button() == Qt::MiddleButton) {
        lastMousePos = event->pos();
        this->setCursor(PAN_CURSOR);
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

void removeFromBoth(std::vector<int>& vec1, std::vector<int>& vec2, int value) {
    auto it = std::find(vec1.begin(), vec1.end(), value);
    if (it != vec1.end()) {
        size_t index = std::distance(vec1.begin(), it);
        vec1.erase(it);
        if (index < vec2.size()) {
            vec2.erase(vec2.begin() + index);
        }
    }
}

void removeFromBothByFirst(std::vector<int>& vec1, std::vector<int>& vec2, int value) {
    for (size_t i = 0; i < vec1.size(); ++i) {
        if (vec1[i] == value) {
            vec1.erase(vec1.begin() + i);
            vec2.erase(vec2.begin() + i);
            return;
        }
    }
}

void Canvas::deleteEdge(int id) {
    Edge *edge = edges.at(id);
    Vertex *start = vertices.at(edge->startId);
    Vertex *end = vertices.at(edge->endId);

    removeFromBothByFirst(start->out.edgeId, start->out.vertexId, id);
    removeFromBothByFirst(end->in.edgeId, end->in.vertexId, id);

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

void Canvas::delay(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

void Canvas::weightsToInf(Vertex& startVertex, std::vector<int>& unchecked) {
    unchecked.push_back(startVertex.id);
    startVertex.weight = INF;

    for (int id : startVertex.out.vertexId) {
        if (contains(unchecked, id)) {
            continue;
        }

        Vertex *vertex = vertices.at(id);
        weightsToInf(*vertex, unchecked);
    }
}

int Canvas::getMinWeightVertex(std::vector<int> vertexIds) {
    int minId = vertexIds[0];
    qreal minWeight = vertices.at(minId)->weight;

    for (int id : vertexIds) {
        qreal weight = vertices.at(id)->weight;

        if (weight == INF) continue;
        if (weight < minWeight) {
            minId = id;
            minWeight = weight;
        }
    }

    return minId;
}

void Canvas::dijkstraAlgorithm(Vertex &startVertex, std::vector<int>& checked) {
    dCurrVertex = startVertex.id;

    for (int id : startVertex.in.edgeId) {
        if (contains(dCheckedEdges, id)) continue;

        if (!contains(checked, edges.at(id)->startId)) continue;

        delay(EDGE_STEP_DELAY_MS);

        if (!isDijkstraRunning) return;
        dCheckedEdges.push_back(id);
        update();
    }

    // Setting weights of neighboring vertices
    for (size_t i = 0; i < startVertex.out.vertexId.size(); ++i) {
        Vertex *vertex = vertices.at(startVertex.out.vertexId[i]);
        Edge *edge = edges.at(startVertex.out.edgeId[i]);

        delay(STEP_DELAY_MS);

        if (!isDijkstraRunning) return;
        bool isCheckedVertex = contains(checked, vertex->id);

        dCheckedEdges.push_back(edge->id);
        dCheckedVertices.push_back(vertex->id);
        update();

        if (!isCheckedVertex) {
            qreal distToVertex = startVertex.weight + edge->weight;
            if (vertex->weight > distToVertex || vertex->weight == INF) {
                vertex->weight = distToVertex;
            }

            delay(STEP_DELAY_MS);

            if (!isDijkstraRunning) return;
            dCheckedEdges.pop_back();
            dCheckedVertices.pop_back();
            update();
        }
    }

    checked.push_back(dCurrVertex);
    dUnchecked.erase(std::remove(dUnchecked.begin(), dUnchecked.end(), dCurrVertex), dUnchecked.end());

    while (dUnchecked.size() != 0) {
        Vertex *vertex = vertices.at(getMinWeightVertex(dUnchecked));

        delay(STEP_DELAY_MS);

        if (!isDijkstraRunning) return;
        dijkstraAlgorithm(*vertex, checked);
    }
}

void Canvas::showHide(std::vector<int>& vec, const std::vector<int> ref) {
    delay(FLICK_DELAY_MS);
    vec = ref;
    update();

    delay(FLICK_DELAY_MS);
    vec.clear();
    update();
}

void Canvas::dijkstraEndAnimation(const std::vector<int> checked) {
    for (int id : checked) {
        if (!isDijkstraRunning) return;
        dEndAnim.push_back(id);

        delay(END_DELAY_MS);
        update();
    }

    std::vector<int> save = dEndAnim;
    for(int i = 0; i < 3; ++i) {
        showHide(dEndAnim, save);
    }
}

void Canvas::Dijkstra(Vertex &startVertex) {
    isDijkstraRunning = true;
    dFirst = startVertex.id;

    weightsToInf(startVertex, dUnchecked);
    startVertex.weight = 0;
    update();

    delay(START_DELAY_MS);

    if (!isDijkstraRunning) return;
    std::vector<int> dijkstraChecked;
    dijkstraAlgorithm(startVertex, dCheckedVertices);

    dEnd = dCurrVertex;
    dijkstraEndAnimation(dCheckedVertices);

    update();
    isDijkstraRunning = false;
}

void Canvas::cancelDijkstra() {
    isDijkstraRunning = false;
    dCheckedVertices.clear();
    dCheckedEdges.clear();
    dUnchecked.clear();
    dEndAnim.clear();
    dFirst = -1;
    dEnd = -1;
    dCurrVertex = -1;
}

void Canvas::keyPressEvent(QKeyEvent *event) {
    int key = event->key();
    int nativeScanCode = event->nativeScanCode();

    if (key == Qt::Key_Return || key == Qt::Key_E) {
        if (!intPressed1.size()) return;
        if (selectedVertices.size() != 2) return;

        linkVertices(selectedVertices[0], selectedVertices[1], getNumFromArray(intPressed1) / (floatExponent1 ? floatExponent1 : 1.f));
        if (intPressed2.size()) linkVertices(selectedVertices[1], selectedVertices[0], getNumFromArray(intPressed2) / (floatExponent2 ? floatExponent2 : 1.f));

        selectedEdges.clear();
        deselectFirstVertex();
    }

    if (key >= '0' && key <= '9') {
        if (selectedVertices.size() != 2) return;
        if (contains(vertices.at(selectedVertices[0])->out.vertexId, selectedVertices[1])) return;
        if (isDijkstraRunning) return;

        if (isFirstLink) {
            if (intPressed1.size() == 0 && key == 0 ) return;
            if (intPressed1.size() == 6) return;

            intPressed1.push_back(key - '0');
            if (floatExponent1) floatExponent1 *= 10;
        }
        else {
            if (contains(vertices.at(selectedVertices[1])->out.vertexId, selectedVertices[0])) return;
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

    if (key == Qt::Key_V || nativeScanCode == 47) {
        currentTool = selectTool;
        this->setCursor(currentTool->getCursor());
        return;
    }

    if (key == Qt::Key_B || nativeScanCode == 48) {
        currentTool = penTool;
        this->setCursor(currentTool->getCursor());
        return;
    }

    if (key == Qt::Key_F || nativeScanCode == 33) {
        for (const auto& [id, vertex] : vertices) {
            vertex->weight = -2;
        }

        cancelDijkstra();

        if (selectedVertices.size() != 1) {
            update();
            return;
        }

        Vertex *startVertex = vertices.at(selectedVertices[0]);
        selectedEdges.clear();
        deselectAllVertices();
        Dijkstra(*startVertex);
    }

    if (key == Qt::Key_D || nativeScanCode == 32) {
        deselectAllVertices();
    }

    if (key == Qt::Key_A || nativeScanCode == 30) {
        deselectAllVertices();

        for (auto& [id, vertex] : vertices) {
            selectVertex(id);
        }

        update();
        return;
    }

    if (key == Qt::Key_Z || nativeScanCode == 18) {
        if (isDijkstraRunning) return;
        if (selectedVertices.size() >= 2) {
            for (int i = 0; i < selectedVertices.size(); ++i) {
                for (int j = i + 1; j < selectedVertices.size(); ++j) {
                    linkVertices(selectedVertices[i], selectedVertices[j], 1);
                }
            }
        }
        qDebug() << "Linked";
    }

    if (key == Qt::Key_Delete) {
        if (selectedVertices.size() <= 0 && selectedEdges.size() <= 0) return;
        if (isDijkstraRunning) return;

        for (int id : selectedEdges) {
            deleteEdge(id);
        }
        selectedEdges.clear();

        for (int id : selectedVertices) {
            deleteVertex(id);
        }
        selectedVertices.clear();
    }

    resetInputState();

    update();
}
