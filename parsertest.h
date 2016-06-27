#ifndef PARSERTEST_H
#define PARSERTEST_H

#include <QMainWindow>

namespace Ui {
class parsertest;
}

class parsertest : public QMainWindow
{
    Q_OBJECT

public:
    explicit parsertest(QWidget *parent = 0);
    ~parsertest();

private:
    Ui::parsertest *ui;
    void loadTextFile();
    void loadTree();
};

#endif // PARSERTEST_H
