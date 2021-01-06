#ifndef PIEVIEW_H
#define PIEVIEW_H

#include <QAbstractItemView> //视图基本功能

class PieView : public QAbstractItemView
{
    Q_OBJECT

public:
    PieView(QWidget *parent = nullptr);

    //得到圆右边彩条文字的绘制范围矩形
    QRect visualRect(const QModelIndex &index) const override;

    //如有必要，滚动视图确保索引处的项可见。视图根据给定的提示定位项目
    //每当有模型索引时都会触发该函数，也就是只要点击视口中的任意模型项，空白位置不会。2：拉动拆分器把右边的圆视口整到只能显示圆时，在表中点击第一列的颜色和名字，右边的圆视图会显示到彩条位置。而把该函数里面if代码注释则不会跳转显示到彩条，这就是确保索引处的项可见。
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

    //返回项在视口坐标点的模型索引。也就是鼠标点击处的模型索引
    QModelIndex indexAt(const QPoint &point) const override;

protected slots:
    //当项在模型中发生更改时，将调用此槽
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;

    //行被插入时调用。把模型中的数据插入视图
    void rowsInserted(const QModelIndex &parent, int start, int end) override;

    //当行将删除行后，右边的圆视图条目会减少
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

protected:
    //开始编辑与给定索引对应的项。点击视图中的任意项都会触发这函数
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;

    //返回索引项的上下左右的位置。这个函数在此项目中应该没用到
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;

    //返回视图的水平偏移量。点击视图中的任意位置都会触发这函数
    int horizontalOffset() const override;
    //视图垂直偏移量。点击视图中的任意位置都会触发这函数
    int verticalOffset() const override;

    //索引所引用的项隐藏在视图中则为true。在此项目中无效
    bool isIndexHidden(const QModelIndex &index) const override;

    //点击圆中的份额触发此函数。决定选中圆部分的绘制方式
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;

    //当光标在小部件内时，按下鼠标按钮时。
    void mousePressEvent(QMouseEvent *event) override;

    //当鼠标移动事件被发送到小部件时
    void mouseMoveEvent(QMouseEvent *event) override;

    //释放鼠标后调用改函数。 这个三个鼠标事件用于在界面拖动显示一个矩形
    void mouseReleaseEvent(QMouseEvent *event) override;

    //绘制圆和旁边的彩色条
    void paintEvent(QPaintEvent *event) override;
    //接收在event参数中传递的小部件调整大小事件。当调用resizeEvent()时，小部件已经有了新的几何形状。
    void resizeEvent(QResizeEvent *event) override;

    //拖动滚动条时
    void scrollContentsBy(int dx, int dy) override;

    //返回"所选项"选择的剪辑区域。
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;


private:
    //得到圆右边彩条文字的绘制范围矩形
    QRect itemRect(const QModelIndex &item) const;
    //圆形被选择的区域，返回指定一个剪辑区域。
    QRegion itemRegion(const QModelIndex &index) const;
    //返回给定父节点下的行数
    int rows(const QModelIndex &index = QModelIndex()) const;
    //设置滚动条。窗口拉小时滚动条就会显示出来
    void updateGeometries() override;

    //圆与左右两边物体的间距,通过控制圆的大小来实现。圆变小后右边的彩色条和字体会变高
    int margin = 10; 
    int totalSize = 300; //圆的直径
    int pieSize = totalSize - 2 * margin; //圆的最终大小直径
    int validItems = 0; //有多少条数据
    double totalValue = 0.0; //总值

    //QRubberBand类提供了一个矩形或直线，可以指示选择或边界。
    QRubberBand *rubberBand = nullptr;
    QPoint origin; //小部件的位置

};

#endif // PIEVIEW_H
