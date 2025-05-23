#include "canvas.h"
#include "dijkstra.h"

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

    // for (int i = 0; i < 300; ++i) createVertex({0, i * 10.f}, VERTEX_RADIUS);

    createVertex( QPointF(226,473) , 25 );
    createVertex( QPointF(259,305) , 25 );
    linkVertices( 0 , 1 , 6 );
    createVertex( QPointF(504,240) , 25 );
    linkVertices( 1 , 2 , 3 );
    createVertex( QPointF(656,313) , 25 );
    linkVertices( 2 , 3 , 2 );
    createVertex( QPointF(629,502) , 25 );
    linkVertices( 3 , 4 , 3 );
    createVertex( QPointF(439,557) , 25 );
    linkVertices( 0 , 5 , 2 );
    linkVertices( 5 , 4 , 8 );
    createVertex( QPointF(455,401) , 25 );
    linkVertices( 0 , 6 , 6 );
    linkVertices( 1 , 6 , 3 );
    linkVertices( 6 , 4 , 9 );
    linkVertices( 6 , 3 , 12.5 );
    linkVertices( 2 , 0 , 25 );
    linkVertices( 4 , 6 , 1 );
    linkVertices( 5 , 6 , 1 );

}

bool contains(std::vector<int> vector, int value) {
    return std::find(vector.begin(), vector.end(), value) != vector.end();
}

int absCeil(qreal value) {
    return value >= 0 ? ceil(value) : floor(value);
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

void delay(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

Vertex* Canvas::getVertex(int id) {
    return vertices.at(id);
}

QPointF Canvas::getScreenCenter() {
    return screenCenter;
}

qreal Canvas::getHalfScreenDiagonal() {
    return halfScreenDiagonal;
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
    QSize mainWindowSize = mainWindow->size();
    return {mainWindowSize.rwidth() / 2.0f, mainWindowSize.rheight() / 2.0f};
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

// int Canvas::getMinWeightVertex(std::vector<int> vertexIds) {
//     int minId = vertexIds[0];
//     qreal minWeight = vertices.at(minId)->weight;

//     for (int id : vertexIds) {
//         qreal weight = vertices.at(id)->weight;

//         if (weight == INF) continue;
//         if (weight < minWeight) {
//             minId = id;
//             minWeight = weight;
//         }
//     }

//     return minId;
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

void Canvas::linkVertices(int firstId, int secondId, qreal weight) {
    if (contains(vertices.at(firstId)->out.vertexId, secondId)) {
        qDebug() << "Already connected";
        return;
    }

    vertices.at(secondId)->in.vertexId.push_back(firstId);
    vertices.at(secondId)->in.edgeId.push_back(totalEdges);
    vertices.at(firstId)->out.vertexId.push_back(secondId);
    vertices.at(firstId)->out.edgeId.push_back(totalEdges);

    edges.insert({totalEdges, new Edge(QString::number(weight), totalEdges, firstId, secondId, weight)});

    ++totalEdges;

    update();

    // qDebug() << "linkVertices(" << firstId << "," << secondId << "," << weight << ");";
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

    painter.setBrush(Qt::green);
    painter.setPen(Qt::green);
    painter.setOpacity(0.3);

    fakeEdge->startId = selectedVertices[0];
    fakeEdge->endId = selectedVertices[1];
    qreal weight = getNumFromArray(intPressed1) / (floatExponent1 ? floatExponent1 : 1.f);
    fakeEdge->displayText = QString::number(weight) + (floatExponent1 == 1 && isFirstLink ? "." : "");
    fakeEdge->draw(this, painter, !isFirstLink);

    if (isFirstLink) return;

    fakeEdge->startId = selectedVertices[1];
    fakeEdge->endId = selectedVertices[0];

    if (intPressed2.size() <= 0) {
        fakeEdge->weight = -1;
        fakeEdge->displayText = "";
    }
    else {
        weight = getNumFromArray(intPressed2)  / (floatExponent2 ? floatExponent2 : 1.f);
        fakeEdge->displayText = QString::number(weight) + (floatExponent2 == 1 ? "." : "");
    }
    fakeEdge->draw(this, painter, true);
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
                Vertex* start = vertices.at(edge->startId);
                Vertex* end = vertices.at(edge->endId);
                QLineF edgeLine = {start->pos, end->pos};
                QLineF normal(clickPos, {0, 0});
                normal.setAngle(edgeLine.angle() + 90);

                QFontMetrics fontMetrics(font);
                QPointF textPos;
                qreal closestDist = edge->distanceToPoint(fontMetrics, &textPos, edgeLine,  normal, start, end, clickPos);

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
        isDijkstraRunning = true;

        Dijkstra algorithm;
        Events events = algorithm.run(*startVertex, vertices, edges);

        for (Event event : events) {
            if (!isDijkstraRunning) break;

            if (event.name == SET_START_VERTEX) {
                qDebug() << "SET_START_VERTEX — Vertex ID:" << event.vertexId;
                dFirst = event.vertexId;
            }
            else if (event.name == SET_CURRENT_VERTEX) {
                qDebug() << "SET_CURRENT_VERTEX — Vertex ID:" << event.vertexId;
                dCurrVertex = event.vertexId;
            }
            else if (event.name == SET_END_VERTEX) {
                qDebug() << "SET_END_VERTEX — Vertex ID:" << event.vertexId;
                dEnd = event.vertexId;
            }
            else if (event.name == CHECK_VERTEX) {
                qDebug() << "CHECK_VERTEX — Vertex ID:" << event.vertexId;
                dCheckedVertices.push_back(event.vertexId);
            }
            else if (event.name == CHECK_EDGE) {
                qDebug() << "CHECK_EDGE — Edge ID:" << event.edgeId;
                dCheckedEdges.push_back(event.edgeId);
            }
            else if (event.name == UNCHECK_VERTEX) {
                qDebug() << "UNCHECK_VERTEX — Vertex ID:" << event.vertexId;
                dCheckedVertices.erase(std::remove(dCheckedVertices.begin(), dCheckedVertices.end(), event.vertexId), dCheckedVertices.end());
            }
            else if (event.name == UNCHECK_EDGE) {
                qDebug() << "UNCHECK_EDGE — Edge ID:" << event.edgeId;
                dCheckedEdges.erase(std::remove(dCheckedEdges.begin(), dCheckedEdges.end(), event.edgeId), dCheckedEdges.end());
            }
            else if (event.name == SET_WEIGHT) {
                qDebug() << "SET_WEIGHT — Vertex ID:" << event.vertexId;
                vertices.at(event.vertexId)->weight = event.weight;
            }
            else {
                qDebug() << "Unknown event";
            }

            delay(300);
            update();
        }
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
