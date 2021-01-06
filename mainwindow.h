#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE //开始命名空间(避免出现重命名)
class QAbstractItemModel; //模型标准接口，抽象
class QAbstractItemView; //视图类基本功能，抽象
QT_END_NAMESPACE //结束命名空间

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void openFile(); //选择文件
    void saveFile(); //保存文件

private:
    void setupModel(); //创建模型
    void setupViews(); //创建视图
    void loadFile(const QString &path); //处理打开文件

    QAbstractItemModel *model = nullptr;
    QAbstractItemView *pieChart = nullptr;

};

#endif // MAINWINDOW_H
