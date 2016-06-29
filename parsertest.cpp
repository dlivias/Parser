#include "parsertest.h"
#include "ui_parsertest.h"
#include "parser.h"
#include <QFile>
#include <QTextStream>

parsertest::parsertest(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::parsertest)
{
    ui->setupUi(this);

    loadTextFile();
    loadTree();
}

parsertest::~parsertest()
{
    delete ui;
}

void parsertest::loadTextFile()
{
    QFile inputFile(":/input.txt");
    inputFile.open(QIODevice::ReadOnly);

    QTextStream in(&inputFile);
    QString line(in.readAll());
    inputFile.close();

    ui->fileEdit->setPlainText(line);
}

void parsertest::loadTree()
{
     QString inputText(ui->fileEdit->toPlainText());
     std::string inputTextStdString = inputText.toStdString();
     ParserTree tree(inputTextStdString);
     if (tree.createTree())
     {
         QString outText = QString::fromStdString(tree.outResult());
         ui->parserEdit->setPlainText(outText);
     }
     else
         ui->parserEdit->setPlainText("Error: can\'t create tree:\n" + QString::fromStdString(tree.error_discription));
}

