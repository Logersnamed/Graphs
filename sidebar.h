#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QDockWidget>
#include <QLineEdit>

class SideBar : public QDockWidget {
    Q_OBJECT

public:
    SideBar(QWidget *parent = nullptr);
    void setText(const QString &text);

signals:
    void textChanged(const QString &newText);

private:
    QLineEdit *textEdit;
};

#endif // SIDEBAR_H
