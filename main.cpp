#include <QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[]){

    //指定的资源文件夹在程序启动时加载
    Q_INIT_RESOURCE(chart);

    QApplication app(argc,argv);
    MainWindow window;
    window.show();

    return app.exec();
}
