#include "canvas.h"
#include "vertex.h"
#include "edge.h"
#include "mainwindow.h"

#include <unordered_map>

#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QPainterPath>

typedef std::unordered_map<int, Vertex*> vertexMap;
typedef std::unordered_map<int, Edge*> edgeMap;
typedef std::unordered_map<int, int> intMap;

Canvas::Canvas(MainWindow *parentWindow, QWidget *parent) : QWidget(parent), mainWindow(parentWindow) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setFocus();
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
        selectedVertices.clear();
    }
    else if (selectedVertices.size() == 2) {
        selectedVertices.erase(selectedVertices.begin());
    }

    selectedVertices.push_back(totalVertices);
    ++totalVertices;

    update();
}

bool Canvas::isInMap(const intMap map, int key) {
    return map.find(key) != map.end();
}

bool isContainsFirst(const std::vector<std::pair<int, int>>& vec, int value) {
    for (const auto& p : vec) {
        if (p.first == value) {
            return true;
        }
    }
    return false;
}

bool isContainsSecond(const std::vector<std::pair<int, int>>& vec, int value) {
    for (const auto& p : vec) {
        if (p.second == value) {
            return true;
        }
    }
    return false;
}

bool isContain(std::vector<int> vector, int value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

void Canvas::linkVertices(int firstId, int secondId, int value) {
    if (isContain(vertices.at(firstId)->out.vertexId, secondId)) {
        qDebug() << "Already connected";
        return;
    }

    vertices.at(secondId)->in.vertexId.push_back(firstId);
    vertices.at(secondId)->in.edgeId.push_back(totalEdges);
    vertices.at(firstId)->out.vertexId.push_back(secondId);
    vertices.at(firstId)->out.edgeId.push_back(totalEdges);

    edges.insert({totalEdges, new Edge(totalEdges, firstId, secondId, value)});

    ++totalEdges;

    update();
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
        qreal distToCenter = QLineF{screenCenter, vertex->pos}.length();
        if (distToCenter - vertex->radius - LINE_THICKNESS > halfScreenDiagonal) {
            continue;
        }

        if (isVertexSelected(id)) {
            painter.setPen(Qt::green);
        }

        if (vertex->weight > -2) {
            bool isChecked = std::find(dCheckedVertices.begin(), dCheckedVertices.end(), id) != dCheckedVertices.end();
            if (id == dEnd) {
                painter.setBrush(dEndColor);
            }
            else if (id == dCurrVertex) {
                painter.setBrush(dCurrColor);
            }
            else if (id == dFirst) {
                painter.setBrush(dFirstColor);
            }
            else if (isChecked) {
                painter.setBrush(dChekcedColor);
            }

        }

        painter.drawEllipse(vertex->pos, vertex->radius, vertex->radius);
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        font.setItalic(false);
        painter.setFont(font);

        QPointF textPos = vertex->pos + getTextCenterAlign(painter.fontMetrics(), vertex->displayName);

        painter.drawText(textPos, vertex->displayName);

        if (vertex->weight > -2) {
            painter.setPen(dWeightColor);
            QFont weightFont = font;
            weightFont.setItalic(true);
            painter.setFont(weightFont);

            QString weightText = vertex->weight == -1 ? "∞" : QString::number(vertex->weight);
            painter.drawText(vertex->pos + WEIGHT_TEXT_OFFSET + getTextCenterAlign(painter.fontMetrics(), weightText), weightText);
            painter.setPen(Qt::black);
        }
    }
}

bool Canvas::isEdgeSelected(const Edge& edge) {
    std::unordered_set<int> selectedSet(selectedVertices.begin(), selectedVertices.end());
    return selectedSet.count(edge.startId) && selectedSet.count(edge.endId);
}

QPointF getShiftAccordingAngle(const QLineF& line, const QPointF& absShift) {
    qreal length = line.length();
    qreal sin = line.dy() / length;
    qreal cos = line.dx() / length;

    return {absShift.x() * sin, absShift.y() * -cos};
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

void Canvas::drawEdge(QPainter& painter, Edge *edge, bool isForceBoth) {
    QPointF startPos = vertices.at(edge->startId)->pos;
    QPointF endPos = vertices.at(edge->endId)->pos;
    int startId = edge->startId;
    int endId = edge->endId;
    QLineF edgeLine = {startPos, endPos};

    QLineF normal(screenCenter, {0, 0});
    normal.setAngle(edgeLine.angle() + 90);

    if (isContain(vertices.at(startId)->in.vertexId, endId) || isForceBoth) {
        QPointF bothShift{0, 0};
        bothShift = newVector(normal, EDGE_BOTH_SHIFT / 2);
        startPos += bothShift;
        endPos += bothShift;
        edgeLine = {startPos, endPos};
    }

    QString text = QString::number(edge->weight);
    QPointF textCenterOffset = getTextCenterAlign(painter.fontMetrics(), text);
    QPointF shift = newVector(normal, {EDGE_TEXT_SHIFT - textCenterOffset.x(), EDGE_TEXT_SHIFT + textCenterOffset.y()});
    QPointF textPos = edgeLine.center() + textCenterOffset + shift;

    qreal closestDist = std::min(QLineF{screenCenter, closestPoint(edgeLine, normal, screenCenter)}.length(),
                                 QLineF{screenCenter, closestPoint(text, textPos, textCenterOffset, screenCenter)}.length());
    if (closestDist - LINE_THICKNESS > halfScreenDiagonal) return;

    std::unordered_set<int> selectedSet(selectedVertices.begin(), selectedVertices.end());
    bool isSelected = selectedSet.count(startId) && selectedSet.count(endId) || std::find(selectedEdges.begin(), selectedEdges.end(), edge->id) != selectedEdges.end();
    if (isSelected) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::green);
    }

    bool isChecked = std::find(dCheckedEdges.begin(), dCheckedEdges.end(), edge->id) != dCheckedEdges.end();
    if (isChecked) {
        painter.setBrush(dChekcedColor);
        painter.setPen(dChekcedColor);
    }

    painter.drawLine(edgeLine);
    drawArrow(painter, {endPos, startPos}, vertices.at(endId)->radius);

    painter.setPen(Qt::black);

    if (isSelected) {
        painter.setBrush(Qt::darkGreen);
        painter.setPen(Qt::darkGreen);
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
        drawEdge(painter, edge, isBoth);
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
    fakeEdge->weight = getNumFromArray(intPressed1);
    drawEdge(painter, fakeEdge, intPressed2.size() > 0);

    if (intPressed2.size()<= 0) return;

    fakeEdge->startId = selectedVertices[1];
    fakeEdge->endId = selectedVertices[0];
    fakeEdge->weight = getNumFromArray(intPressed2);
    drawEdge(painter, fakeEdge, intPressed2.size() > 0);
}

void Canvas::drawDebug(QPainter& painter) {
    painter.setBrush(Qt::yellow);
    painter.setPen(Qt::yellow);

    for (auto& line : debugLines) {
        painter.drawLine(line);
    }

    painter.setBrush(Qt::blue);
    painter.setPen(Qt::blue);

    for (auto& pos : debugPoints) {
        painter.drawEllipse(pos, 3, 3);
    }

    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);
}

void Canvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.translate(offset);
    painter.scale(scaleFactor, scaleFactor);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font);

    QPointF center = getAbsoluteCenter();
    screenCenter = (center - offset) / scaleFactor;
    halfScreenDiagonal = qSqrt(QPointF::dotProduct(center, center)) / scaleFactor;

    drawGrid(painter, center);
    drawEdges(painter);
    drawFakeEdges(painter);
    drawVertices(painter);

    drawDebug(painter);
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    QPointF clickPos = getTransformedPos(event->pos());

    if (event->button() == Qt::LeftButton) {
        intPressed1.clear();
        intPressed2.clear();
        isFirstLink = true;

        if (currentTool == selectTool) {
            bool isShiftModifier = (event->modifiers() == Qt::ShiftModifier);
            if (!isShiftModifier) {
                selectedVertices.clear();
                selectedEdges.clear();
            }

            bool isVertexClicked = false;
            for (const auto& [id, vertex] : vertices) {
                bool isInRadius = QLineF(clickPos, vertex->pos).length() <= vertex->radius;
                if (!isInRadius) continue;

                if (!isShiftModifier) selectedVertices.clear();
                isVertexClicked = true;

                bool isSelected = std::find(selectedVertices.begin(), selectedVertices.end(), id) != selectedVertices.end();
                if (!isSelected) {
                    selectedVertices.push_back(id);
                }

                draggingVertex = vertex;
                draggingOffset = vertex->pos - clickPos;
            }

            if (!isVertexClicked) {
                QPointF center = getAbsoluteCenter();
                screenCenter = (center - offset) / scaleFactor;
                halfScreenDiagonal = qSqrt(QPointF::dotProduct(center, center)) / scaleFactor;

                qreal closest = - 1;
                int closestId = -1;
                for (const auto& [id, edge] : edges) {
                    QPointF startPos = vertices.at(edge->startId)->pos;
                    QPointF endPos = vertices.at(edge->endId)->pos;
                    QLineF edgeLine = {startPos, endPos};

                    QLineF normal(clickPos, {0, 0});
                    normal.setAngle(edgeLine.angle() + 90);

                    if (isContain(vertices.at(edge->startId)->in.vertexId, edge->endId)) {
                        QPointF bothShift{0, 0};
                        bothShift = newVector(normal, EDGE_BOTH_SHIFT / 2);
                        startPos += bothShift;
                        endPos += bothShift;
                        edgeLine = {startPos, endPos};
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

                    if (!isShiftModifier) selectedEdges.clear();
                }

                if (closestId != -1) selectedEdges.push_back(closestId);
            }

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

void Canvas::deleteEdge(int id) {
    Edge *edge = edges.at(id);
    Vertex *start = vertices.at(edge->startId);
    Vertex *end = vertices.at(edge->endId);

    removeFromBoth(start->out.edgeId, start->out.vertexId, id);
    removeFromBoth(end->in.edgeId, end->in.vertexId, id);

    delete edges.at(id);
    edges.erase(id);
}

void Canvas::deleteVertex(int id) {
    for (const auto& [edgeId, edge] : edges) {
        if (edge->startId == id || edge->endId == id) {
            deleteEdge(edgeId);
        }
    }

    delete vertices.at(id);
    vertices.erase(id);
}

#include <QEventLoop>
#include <QTimer>

void Canvas::delay(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

void Canvas::weightsToInf(Vertex& startVertex, std::vector<int>& checked) {
    checked.push_back(startVertex.id);
    startVertex.weight = -1;

    for (int id : startVertex.out.vertexId) {
        if (std::find(checked.begin(), checked.end(), id) != checked.end()) {
            continue;
        }

        Vertex *vertex = vertices.at(id);
        weightsToInf(*vertex, checked);
    }
}

std::vector<std::pair<int, int>> Canvas::sortByEdgeWeights(std::vector<int> vertexIds, std::vector<int> edgeIds) {
    std::vector<std::pair<int, int>> vertexEdgePairs;

    for (size_t i = 0; i < vertexIds.size(); ++i) {
        vertexEdgePairs.emplace_back(vertexIds[i], edgeIds[i]);
    }

    std::sort(vertexEdgePairs.begin(), vertexEdgePairs.end(),
              [this](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return edges.at(a.second)->weight < edges.at(b.second)->weight;
              });

    return vertexEdgePairs;
}


void Canvas::dijkstraAlgorithm(Vertex &startVertex, std::vector<int>& checked) {
    dCurrVertex = startVertex.id;

    for (size_t i = 0; i < startVertex.out.vertexId.size(); ++i) {
        Vertex *vertex = vertices.at(startVertex.out.vertexId[i]);
        Edge *edge = edges.at(startVertex.out.edgeId[i]);

        if (std::find(checked.begin(), checked.end(), vertex->id) != checked.end()) {
            continue;
        }

        delay(STEP_DELAY_MS);
        update();

        dCheckedEdges.push_back(edge->id);
        dCheckedVertices.push_back(vertex->id);

        int distToVertex = startVertex.weight + edge->weight;
        if (vertex->weight > distToVertex || vertex->weight == -1) {
            vertex->weight = distToVertex;
        }

        delay(STEP_DELAY_MS);
        update();

        dCheckedEdges.pop_back();
        dCheckedVertices.pop_back();
    }

    checked.push_back(startVertex.id);

    std::vector<std::pair<int, int>> vertexEdgePairs = sortByEdgeWeights(startVertex.out.vertexId, startVertex.out.edgeId);

    for (const auto& [vertexId, edgeId] : vertexEdgePairs) {
        Vertex *vertex = vertices.at(vertexId);

        dCheckedEdges.push_back(edgeId);

        if (std::find(checked.begin(), checked.end(), vertexId) != checked.end()) {
            continue;
        }

        dijkstraAlgorithm(*vertex, checked);

        delay(STEP_DELAY_MS);
        update();
    }
}

void Canvas::Dijkstra(Vertex &startVertex) {
    // block input

    dFirst = startVertex.id;

    std::vector<int> checked;
    weightsToInf(startVertex, checked);
    startVertex.weight = 0;
    update();

    delay(STEP_DELAY_MS + 1000);

    std::vector<int> dijkstraChecked;
    dijkstraAlgorithm(startVertex, dCheckedVertices);

    dEnd = dCurrVertex;

    update();
}

void Canvas::keyPressEvent(QKeyEvent *event) {
    int key = event->key();
    int nativeScanCode = event->nativeScanCode();

    if (key == Qt::Key_Return || key == Qt::Key_E) {
        if (!intPressed1.size()) return;
        if (selectedVertices.size() != 2) return;

        linkVertices(selectedVertices[0], selectedVertices[1], getNumFromArray(intPressed1));
        if (!isFirstLink) linkVertices(selectedVertices[1], selectedVertices[0], getNumFromArray(intPressed2));

        selectedEdges.clear();
        selectedVertices.erase(selectedVertices.begin());
    }

    if (key >= '0' && key <= '9') {
        if (selectedVertices.size() != 2) return;
        if (isContain(vertices.at(selectedVertices[0])->out.vertexId, selectedVertices[1])) return;



        if (isFirstLink) {
            if (intPressed1.size() == 0 && key == 0 ) return;
            intPressed1.push_back(key - '0');
        }
        else {
            if (isContain(vertices.at(selectedVertices[1])->out.vertexId, selectedVertices[0])) return;
            if (intPressed2.size() == 0 && key == 0 ) return;
            intPressed2.push_back(key - '0');
        }

        update();

        return;
    }

    if (key == Qt::Key_Space) {
        isFirstLink = false;

        return;
    }

    if (key == Qt::Key_Backspace) {
        if (!intPressed1.size()) return;

        if (isFirstLink) {
            intPressed1.pop_back();
        }
        else {
            if (!intPressed2.size()) {
                isFirstLink = true;
                return;
            }
            intPressed2.pop_back();
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

        dCheckedVertices.clear();
        dCheckedEdges.clear();
        dFirst = -10;
        dEnd = -10;
        dCurrVertex = -10;

        if (selectedVertices.size() != 1) {

            update();
            return;
        }

        Vertex *startVertex = vertices.at(selectedVertices[0]);
        selectedEdges.clear();
        selectedVertices.clear();
        Dijkstra(*startVertex);
    }

    if (key == Qt::Key_D || nativeScanCode == 32) {
        selectedVertices.clear();
        update();
    }

    if (key == Qt::Key_A || nativeScanCode == 30) {
        selectedVertices.clear();

        for (auto& [id, vertex] : vertices) {
            qDebug() << id;
            selectedVertices.push_back(id);
        }

        update();
        return;
    }

    if (key == Qt::Key_Z || nativeScanCode == 18) {
        if (selectedVertices.size() >= 2) {
            for (int i = 0; i < selectedVertices.size(); ++i) {
                for (int j = i + 1; j < selectedVertices.size(); ++j) {
                    linkVertices(selectedVertices[i], selectedVertices[j], 1);
                }
            }
        }
    }

    if (key == Qt::Key_O || nativeScanCode == 24) {
        debugPoints.clear();
        debugLines.clear();

        update();
        return;
    }

    if (key == Qt::Key_Delete) {
        if (selectedVertices.size() <= 0 && selectedEdges.size() <= 0) return;

        for (int id : selectedEdges) {
            deleteEdge(id);
        }

        selectedEdges.clear();

        for (int id : selectedVertices) {
            deleteVertex(id);
        }

        selectedVertices.clear();

        update();
    }

    intPressed1.clear();
    intPressed2.clear();
    isFirstLink = true;

    update();
}



















