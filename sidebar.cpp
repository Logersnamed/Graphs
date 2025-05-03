#include "sidebar.h"
#include <QVBoxLayout>
#include <QWidget>

SideBar::SideBar(QWidget *parent) : QDockWidget("Настройки текста", parent) {
    QWidget *content = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(content);

    textEdit = new QLineEdit;
    textEdit->setPlaceholderText("Введите текст...");
    layout->addWidget(textEdit);

    content->setLayout(layout);
    setWidget(content);

    connect(textEdit, &QLineEdit::textChanged, this, &SideBar::textChanged);
}

void SideBar::setText(const QString &text) {
    textEdit->setText(text);
}
