#include "maindialog.h"

#include <QApplication>

#pragma execution_character_set("utf-8")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainDialog w;

    // 显示窗口最小化按钮
    w.setWindowFlags(w.windowFlags()|Qt::WindowMinimizeButtonHint);

    w.show();
    return a.exec();
}
