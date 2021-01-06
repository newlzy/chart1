#include "mainwindow.h"
#include <QtWidgets>
#include <pieview.h>
#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent)
{
    //菜单小部件
    QMenu *fileMenu = new QMenu(tr("&文件"),this);
    QAction *openAction = fileMenu->addAction(tr("&打开文件"));
    openAction->setShortcuts(QKeySequence::Open); //打开文件
    QAction *saveAction = fileMenu->addAction(tr("&另存为..."));
    saveAction->setShortcuts(QKeySequence::SaveAs);
    QAction *quitAction = fileMenu->addAction(tr("&退出"));
    quitAction->setShortcuts(QKeySequence::Quit);

    setupModel(); //创建模型
    setupViews(); //创建视图

    //打开文件
    connect(openAction,&QAction::triggered,this,&MainWindow::openFile);
    //保存文件
    connect(saveAction,&QAction::triggered,this,&MainWindow::saveFile);
    connect(quitAction,&QAction::triggered,qApp,&QCoreApplication::quit);
    //将菜单添加到菜单栏
    menuBar()->addMenu(fileMenu);
    statusBar(); //返回主窗口的状态栏
    loadFile(":/Charts/qtdata.cht");
    setWindowTitle(tr("图表"));
    resize(870,560);
}

//选择文件
void MainWindow::openFile()
{
    //用户选择文件目录
    const QString fileName = QFileDialog::getOpenFileName(this,tr("选择一个数据文件"),"*.cht");
    if(!fileName.isEmpty())
        loadFile(fileName); //处理打开的文件
}

//保存文件
void MainWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("保存文件为"),"","*.cht");
    if(fileName.isEmpty()) //文件无数据
        return;

    QFile file(fileName);
    //可写文本
    if(!file.open(QFile::WriteOnly | QFile::Text))
        return;
    QTextStream stream(&file); //为读写文本提供一个方便的接口
    for(int row = 0; row < model->rowCount(QModelIndex()); ++row){
        QStringList pieces;

        //在列表的末尾插入值
        pieces.append(model->data(model->index(row,0,QModelIndex()),Qt::DisplayRole).toString());
        pieces.append(model->data(model->index(row,1,QModelIndex()),Qt::DisplayRole).toString());
        pieces.append(model->data(model->index(row,0,QModelIndex()),Qt::DecorationRole).toString());
        //将所有字符串连接为一个字符串写入到文件
        stream << pieces.join(',')<<"\n";
    }
    file.close();
    statusBar()->showMessage(tr("保存 %1 成功").arg(fileName),2000);
}

//创建模型
void MainWindow::setupModel()
{
    //创建自定义数据的通用模型
    model = new QStandardItemModel(8,2,this);
    model->setHeaderData(0, Qt::Horizontal, tr("标签"));
    model->setHeaderData(1,Qt::Horizontal,tr("数量"));
}

//创建视图
void MainWindow::setupViews()
{
    QSplitter *splitter = new QSplitter; //拆分器
    QTableView *table = new QTableView; //默认表视图
    pieChart = new PieView; //自定义视图，圆
    splitter->addWidget(table); //添加到拆分器的布局中
    splitter->addWidget(pieChart);
    //更新小部件在位置索引处的大小策略，使其具有拉伸因子。参数索引，伸展
    splitter->setStretchFactor(0,0);
    splitter->setStretchFactor(1,1);

    table->setModel(model); //设置显示视图的模型
    pieChart->setModel(model);

    //跟踪视图中或同一模型的多个视图中所选的项。
    QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
    table->setSelectionModel(selectionModel); //设置当前的选择模型
    pieChart->setSelectionModel(selectionModel);

    //为视图提供标题行或标题列。返回视图的水平表头
    QHeaderView *headerView = table->horizontalHeader();
    //最后可见部分是否占用所有可用空间
    headerView->setStretchLastSection(true);

    //将给定的小部件设置为主窗口的中心小部件。
    setCentralWidget(splitter);
}

//处理打开的文件,把文件数据插入到模型中
void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName); //读取文件
    //打开模式为只读文本
    if(!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream stream(&file); //为读写文本提供了一个方便的接口

    //从父级开始删除行
    model->removeRows(0,model->rowCount(QModelIndex()),QModelIndex());
    int row = 0;
    while (!stream.atEnd()) { //没有数据要从QTextStream读取时返回true
        //从流中读取一行文本
        const QString line = stream.readLine();
        if(!line.isEmpty()){
            //插入行到模型中
            model->insertRows(row,1,QModelIndex());
            //在，号的地方拆分成子字符串。字段是空的在结果中不包含
            const QStringList pieces = line.split(QLatin1Char(','),Qt::SkipEmptyParts);
            if(pieces.size() < 3)
                continue;

            //在索引处设置值
            model->setData(model->index(row,0,QModelIndex()),pieces.value(0));
            model->setData(model->index(row,1,QModelIndex()),pieces.value(1));
            //插入颜色,角色以图标的形式作为装饰呈现的数据
            model->setData(model->index(row,0,QModelIndex()),QColor(pieces.value(2)),Qt::DecorationRole);
            row++;
        }
    };

    file.close();
    //状态栏
    statusBar()->showMessage(tr("加载完成 %1").arg(fileName),2000);
}


