#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <QPointF>
#include <QFontMetrics>
#include <QString>

constexpr int INF = -1;
constexpr int UNDEFINED = -2;

namespace utils {
    template <typename Container, typename T>
    bool contains(const Container& container, const T& value) {
        return std::find(container.begin(), container.end(), value) != container.end();
    }

    inline QPointF getTextCenterAlign(QFontMetrics fm, QString text) {
        int width = fm.horizontalAdvance(text);
        int height = fm.ascent() - fm.descent();

        return {-width / 2.0f, height / 2.0f};
    }

    inline int absCeil(qreal value) {
        return value >= 0 ? ceil(value) : floor(value);
    }

    inline void removeFromBothByFirst(std::vector<int>& vec1, std::vector<int>& vec2, int value) {
        for (size_t i = 0; i < vec1.size(); ++i) {
            if (vec1[i] == value) {
                vec1.erase(vec1.begin() + i);
                vec2.erase(vec2.begin() + i);
                return;
            }
        }
    }

}

#endif // UTILS_H
