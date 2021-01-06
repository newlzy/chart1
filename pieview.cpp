#include "pieview.h"
#include <QtWidgets>
#include <qdebug.h>

PieView::PieView(QWidget *parent):QAbstractItemView(parent)
{
    //水平滚动条。关于滚动条要详细看下QAbstractScrollArea类介绍。这个视图类是在此类的视口上做绘制显示。
    //设置值为0时将隐藏滚动条。
    horizontalScrollBar()->setRange(0,0);
    verticalScrollBar()->setRange(0,0); //垂直滚动条
}

//得到圆右边彩条文字的绘制范围矩形。
QRect PieView::visualRect(const QModelIndex &index) const
{
    //得到圆右边彩条文字的绘制范围矩形。
    QRect rect = itemRect(index);
    if(!rect.isValid()) //矩形有效返回true
        return rect;

    //left矩形左边缘的x坐标，得到绘制彩条文字的真正范围坐标
    return QRect(rect.left() - horizontalScrollBar()->value(),rect.top() - verticalScrollBar()->value(),rect.width(),rect.height());
}

//滚动视图确保索引处的项可见。视图根据给定的提示定位项目
//每当有模型索引时都会触发该函数，也就是只要点击视口中的任意模型项，空白位置不会。2：拉动拆分器把右边的圆视口整到只能显示圆时，在表中点击第一列的颜色和名字，右边的圆视图会显示到彩条位置。而把该函数里面if代码注释则不会跳转显示到彩条，这就是确保索引处的项可见。
void PieView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    QRect area = viewport()->rect(); //小部件的内部几何形状
    QRect rect = visualRect(index); //索引处的项在视口上占用的矩形

    //点击第二列的标题时会选中该列中所有行。
    if(rect.left() < area.left()){ //left返回矩形左边缘的x坐标
        //setValue设置滑动块的值。
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + rect.left() - area.left());
    }else if(rect.right() > area.right()){ //返回矩形右边缘的x坐标
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + qMin(rect.right() - area.right(),rect.left() - area.left()));
    }

    //返回矩形上边缘的y坐标
    if(rect.top() < area.top()){
        verticalScrollBar()->setValue(verticalScrollBar()->value() + rect.top() - area.top());
    }else if(rect.bottom() > area.bottom()){ //矩形底边y坐标
        verticalScrollBar()->setValue(verticalScrollBar()->value() + qMin(rect.bottom() - area.bottom(),rect.top() - area.top()));
    }
    update(); //更新小部件
}

//返回项在视口坐标点的模型索引。也就是鼠标点击处的模型索引
QModelIndex PieView::indexAt(const QPoint &point) const
{
    if(validItems == 0) //数据量
        return QModelIndex();

    //鼠标单击处的坐标位置
    int wx = point.x() + horizontalScrollBar()->value();
    int wy = point.y() + verticalScrollBar()->value();

    //if里面的代码得到所选圆的份额索引
    if(wx < totalSize){ //说明没出界
        double cx = wx - totalSize / 2;
        //正cy表示中心以上的项
        double cy = totalSize / 2 - wy;

        //sqrt开平方根.确定距离饼图中心点的距离。
        double d = std::sqrt(std::pow(cx,2) + std::pow(cy,2));
        if(d == 0 || d > pieSize / 2)
            return QModelIndex();

        //确定这个点的角度.qRadiansToDegrees将浮点数转换为角度
        double angle = qRadiansToDegrees(std::atan2(cy,cx));
        if(angle < 0)
            angle = 360 + angle;

        //找到馅饼的相关部分。
        double startAngle = 0.0;

        for(int row = 0; row < model()->rowCount(rootIndex()); ++row){
            QModelIndex index = model()->index(row,1,rootIndex());
            double value = model()->data(index).toDouble();
            if(value > 0.0){
                double sliceAngle = 360 * value / totalValue;
                if(angle >= startAngle && angle < (startAngle + sliceAngle))
                    return model()->index(row,1,rootIndex());

                startAngle += sliceAngle;
            }
        }
    }else{
        //QFontMetrics提供字体度量信息。height返回字体高度
        double itemHeight = QFontMetrics(viewOptions().font).height();
        //得到彩条数量
        int listItem = int((wy - margin) / itemHeight);
        int validRow = 0;

        for(int row = 0; row < model()->rowCount(rootIndex()); ++row){
            QModelIndex index = model()->index(row,1,rootIndex());
            if(model()->data(index).toDouble() > 0.0){
                if(listItem == validRow) //到所选彩条位置时
                    return model()->index(row,0,rootIndex()); //返回所选彩条的位置索引

                //更新与下一个有效行对应的列表索引
                ++validRow;
            }
        }
    }
    return QModelIndex();
}

//当项在模型中发生更改时，将调用此槽
void PieView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QAbstractItemView::dataChanged(topLeft,bottomRight,roles);

    //QVector动态数组模板。contains模板有值则true。DisplayRole以文本形式呈现数据
    if(!roles.contains(Qt::DisplayRole))
        return;

    validItems = 0; //有多少条数据
    totalValue = 0.0; //总值

    //rowCount给定父节点下的行数。rootIndex模型根项索引。
    for(int row = 0; row < model()->rowCount(rootIndex()); ++row){
        //返回模型中由给定行、列和父索引指定的项索引
        QModelIndex index = model()->index(row,1,rootIndex());
        //返回索引项的数据
        double value = model()->data(index,Qt::DisplayRole).toDouble();
        if(value > 0.0){
            totalValue += value;
            validItems++;
        }
    }
    //返回viewport(视口)小部件。更新小部件
    viewport()->update();
}

//行被插入时调用。这函数在此项目中好像作用不大
void PieView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for(int row = start; row <= end; ++row){
        QModelIndex index = model()->index(row,1,rootIndex());
        double value = model()->data(index).toDouble();
        if(value > 0.0){
            totalValue += value;
            ++validItems;
        }
    }
    QAbstractItemView::rowsInserted(parent,start,end);
}

//当行将删除行后，右边的圆视图条目会减少
void PieView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for(int row = start; row <= end; ++row){
        QModelIndex index = model()->index(row,1,rootIndex());
        double value = model()->data(index).toDouble();
        if(value > 0.0){
            totalValue -= value;
            --validItems;
        }
    }
    QAbstractItemView::rowsAboutToBeRemoved(parent,start,end);
}

//开始编辑与给定索引对应的项
bool PieView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    //模型索引所引用的列
    if(index.column() == 0)
        return QAbstractItemView::edit(index,trigger,event);
    else
        return false;
}

//返回索引项的上下左右的位置
QModelIndex PieView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers /*modifiers*/)
{
    QModelIndex current = currentIndex(); //当前项目的模型索引

    //该枚举描述了在项之间导航的不同方法
    switch (cursorAction) {
        case MoveLeft: //移动到当前项目的左侧
    case MoveUp: //移动到当前项目上方的项目
        if(current.row() > 0)
            current = model()->index(current.row()-1,current.column()); //获得指定索引，当前项上方的项
        else
            current = model()->index(0,current.column(),rootIndex());
        break;
    case MoveRight: //移动到当前项目的右侧
    case MoveDown: //移动到当前项目下面的项目
        if(current.row() < rows(current) - 1){  //rows返回给定索引的模型项的父项
            current = model()->index(current.row() + 1,current.column(),rootIndex());
        }else
            current = model()->index(rows(current)-1,current.column(),rootIndex());
        break;
    default:
        break;
    }
    viewport()->update(); //更新小部件
    return current;
}

//返回视图的水平偏移量
int PieView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

//视图垂直偏移量
int PieView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

//索引所引用的项隐藏在视图中则为true
bool PieView::isIndexHidden(const QModelIndex &index) const
{
    return false;
}

//点击圆中的份额触发此函数。决定选中圆部分的绘制方式
void PieView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    //translated返回矩形的副本。normalized返回一个规格化的矩形。这里是把rect的x坐标给horizontalScrollBar()->value()
    QRect contentsRect = rect.translated(horizontalScrollBar()->value(),verticalScrollBar()->value()).normalized();

    int rows = model()->rowCount(rootIndex()); // 所有行数
    int columns = model()->columnCount(rootIndex()); //所有列数
    QModelIndexList indexes; //被选中的项

    for(int row = 0; row < rows; ++row){
        for(int column = 0; column < columns; ++column){
            QModelIndex index = model()->index(row, column, rootIndex());
            //itemRegion是自定义函数，为圆中选中的项。QRegion为绘画指定一个剪辑区域。
            QRegion region = itemRegion(index);
            //区域相交返回true，也就是选择多项圆份额时。
            if(region.intersects(contentsRect))
                indexes.append(index); //在列表的末尾插入值。
        }
    }

    if(indexes.size() > 0){
        //索引位置的项对应的行。
        int firstRow = indexes.at(0).row();
        int lastRow = firstRow;
        //索引位置对应的列
        int firstColumn = indexes.at(0).column();
        int lastColumn = firstColumn;

        //得到绘制每份的起始和终止。
        for(int i = 1; i < indexes.size(); ++i){
            firstRow = qMin(firstRow,indexes.at(i).row()); //小
            lastRow = qMax(lastRow,indexes.at(i).row()); //大
            firstColumn = qMin(firstColumn,indexes.at(i).column());
            lastColumn = qMax(lastColumn,indexes.at(i).column());
        }

        //管理模型中所选项目的信息。
        QItemSelection selection(model()->index(firstRow,firstColumn,rootIndex()),model()->index(lastRow,lastColumn,rootIndex()));

        //selectionModel返回当前选择模型。select使用指定的命令选择模型项索引。command描述了选择模型的更新方式。
        selectionModel()->select(selection,command);
    }
    update();
}

//鼠标按下
void PieView::mousePressEvent(QMouseEvent *event)
{
    QAbstractItemView::mousePressEvent(event);
    origin = event->pos(); //小部件的位置
    if(!rubberBand) //显示新的边界区域，橡皮筋
        rubberBand = new QRubberBand(QRubberBand::Rectangle,viewport());

    //设置橡皮筋的几何形状
    rubberBand->setGeometry(QRect(origin,QSize()));
    rubberBand->show();
}

//鼠标移动事件
void PieView::mouseMoveEvent(QMouseEvent *event)
{
    if(rubberBand)
        rubberBand->setGeometry(QRect(origin,event->pos()).normalized()); //设置橡皮筋几何形状
    QAbstractItemView::mouseMoveEvent(event);

}

//释放鼠标后调用改函数
void PieView::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseReleaseEvent(event);
    if(rubberBand)
        rubberBand->hide(); //小部件可见
    viewport()->update(); //更新视口部件
}

//绘制圆和旁边的彩色条
void PieView::paintEvent(QPaintEvent *event)
{
    //跟踪视图的选中项，或同一模型中的多个视图
    QItemSelectionModel *selections = selectionModel();
    //在视图小部件中绘制项目的参数
    QStyleOptionViewItem option = viewOptions();

    //定义填充模式。palette颜色组。base当前颜色组的基刷
    QBrush background = option.palette.base();
    //画笔。一种一般的前景色。
    QPen foreground(option.palette.color(QPalette::WindowText));

    QPainter painter(viewport()); //在视口中绘制
    painter.setRenderHint(QPainter::Antialiasing); //抗锯齿

    //rect返回需要更新的矩形。
    painter.fillRect(event->rect(),background);
    painter.setPen(foreground); //设置画笔

    //视口矩形。pieRect为圆的直径
    QRect pieRect = QRect(margin,margin,pieSize,pieSize);

    if(validItems <= 0) //没有数据时不进行绘画
        return;

    painter.save(); //保存当前绘制状态
    //平移确定坐标后画圆，用pieRect是看有没有设置边距(margin)
    painter.translate(pieRect.x() - horizontalScrollBar()->value(),pieRect.y() - verticalScrollBar()->value());
    qDebug() << horizontalScrollBar()->value();
    painter.drawEllipse(0,0,pieSize,pieSize); //画圆

    /* 以上的代码只画了一个圆，里面还没有颜色跟分块 。下面代码画圆和填充颜色*/

    double startAngle = 0.0; //开始角度
    int row;
    //遍历父节点下的行数
    for(row = 0; row < model()->rowCount(rootIndex()); ++row){
        QModelIndex index = model()->index(row,1,rootIndex());
        double value = model()->data(index).toDouble(); //数据

        //在有数据的情况下，根据数据来绘制圆的颜色和份数
        if(value > 0.0){
            double angle = 360 * value / totalValue; //每份数据所需要的角度份额，角度
            QModelIndex colorIndex = model()->index(row,0,rootIndex());

            //圆的颜色。DecorationRole要以图标的形式作为装饰呈现的数据。第一列数据的图标是颜色块，所以可以用来填充圆
            QColor color = QColor(model()->data(colorIndex,Qt::DecorationRole).toString());
            qDebug() << model()->data(colorIndex);

            //currentIndex当前项目的模型索引。这里为圆中份额全部选中时
            if(currentIndex() == index){
                //setBrush设置填充颜色和模式
                painter.setBrush(QBrush(color,Qt::Dense4Pattern));
            }
            //跟踪视图选中项，选择了给定的模型索引。选中部分圆份额时。
            else if(selections->isSelected(index)){
                painter.setBrush(QBrush(color,Qt::Dense3Pattern));
            }else
                painter.setBrush(QBrush(color));

            //用指定的宽度和高度以及给定的开始角度和跨度角绘制从(x, y)开始的矩形定义的饼。乘16，要比数据条数多2
            painter.drawPie(0,0,pieSize,pieSize,int(startAngle*16),int(angle*16));
            startAngle += angle;
        }
    }
    painter.restore(); //恢复状态

    /* 下面代码绘制圆右边的色条和文字 */

    int keyNumber = 0; //颜色条绘制的次数
    for(row = 0; row < model()->rowCount(rootIndex());++row){
        QModelIndex index = model()->index(row,1,rootIndex()); //数值数据
        double value = model()->data(index).toDouble();
        if(value > 0.0){
            //rootIndex返回模型根项的模型索引
            QModelIndex labelIndex = model()->index(row,0,rootIndex()); //第一列的数据
            qDebug() << model()->data(labelIndex);
            //在视图小部件中绘制项目的参数
            QStyleOptionViewItem option = viewOptions();

            //得到绘制彩条文字的真正范围坐标
            option.rect = visualRect(labelIndex);

            //跟踪视图选中项，选择了给定的模型索引
            if(selections->isSelected(labelIndex)){
                //绘制基本元素时使用的标志。用于指示是否选择小部件
                option.state |= QStyle::State_Selected;
            }
            if(currentIndex() == labelIndex){ //启动后的第一个项
                //用于指示小部件是否有焦点
                option.state |= QStyle::State_HasFocus;
            }
            //itemDelegate视图和模型使用的项委托。paint是抽象函数，实现自定义项委托。这条代码用来绘制色条和文字。
            itemDelegate()->paint(&painter,option,labelIndex);
            ++keyNumber;
        }
    }
}

void PieView::resizeEvent(QResizeEvent *event)
{
    updateGeometries();
}

//拖动滚动条时
void PieView::scrollContentsBy(int dx, int dy)
{
    viewport()->scroll(dx,dy);
}

//返回给定选择项的视口中的区域。以视口坐标返回与所选内容对应的区域。
//返回"所选项"选择的剪辑区域。
QRegion PieView::visualRegionForSelection(const QItemSelection &selection) const
{
    //selection管理模型中所选项目的信息。count返回列表中项的数量，与size()相同
    int ranges = selection.count();
    if(ranges == 0)
        return QRect();

    QRegion region; //为画家指定一个剪辑区域
    for(int i = 0; i < ranges; ++i){
        //模型中选定项目范围的信息
        const QItemSelectionRange &range = selection.at(i);
        //top返回与选择范围中最上面所选行对应的行索引。bottom最低。
        for(int row = range.top(); row <= range.bottom(); ++row){
            //left最左，right最右
            for(int col = range.left(); col <= range.right(); ++col){
                QModelIndex index = model()->index(row,col,rootIndex());
                region += visualRect(index); //索引处的几何形状
            }
        }
    }
    return region;
}


//功能1：参数索引位置的几何形状。2：得到圆右边彩条文字的绘制范围矩形。
QRect PieView::itemRect(const QModelIndex &index) const
{
    if(!index.isValid()) //模型索引有效为true
        return QRect();

    QModelIndex valueIndex; //第二列的数据

    //传进来的参数会是第几列
    if(index.column() != 1){//column模型索引所引用的列
        //拿到第0列行对应的数值。
        valueIndex = model()->index(index.row(),1,rootIndex());
        qDebug()<<model()->data(valueIndex);
        qDebug()<<model()->data(index);
    }
    else
        valueIndex = index;

    if(model()->data(valueIndex).toDouble() <= 0.0)
        return QRect();

    int listItem = 0; //数据量计数
    //第一次进函数时index.row为1，因为是第二行。为后面绘制彩条文字起作用，每一个彩条绘制一个矩形。
    for(int row = index.row()-1;row >= 0; --row){
        if(model()->data(model()->index(row,1,rootIndex())).toDouble() > 0.0)
            listItem++;
    }

    switch (index.column()) {
    case 0:{
        //字体的高度
        const qreal itemHeight = QFontMetrics(viewOptions().font).height();
        //得到绘制彩条文字的范围矩形。qRound:四舍五入到最接近的整数。这句是本函数的核心代码
        return QRect(totalSize,qRound(margin + listItem * itemHeight),totalSize - margin,qRound(itemHeight));
    }
    case 1:
        return viewport()->rect(); //保存小部件的内部几何形状
    }
    return QRect();
}

//圆形被选择的区域，返回指定一个剪辑区域。
QRegion PieView::itemRegion(const QModelIndex &index) const
{
    if(!index.isValid())
        return QRegion();

    if(index.column() !=1) //索引引用的列。确保选择的一定要是这列
        return itemRect(index); //索引位置的项矩形
    if(model()->data(index).toDouble() <= 0.0)
        return QRegion();

    double startAngle = 0.0; //开始画圆弧的地方
    for(int row  = 0; row < model()->rowCount(rootIndex());++row){
        QModelIndex sliceIndex = model()->index(row,1,rootIndex());
        double value = model()->data(sliceIndex).toDouble();
        if(value > 0.0){
            double angle = 360 * value / totalValue; //角度
            if(sliceIndex == index){ //找到选中的数值时
                QPainterPath slicePath; //绘图容器
                //将当前位置移动到x，y
                slicePath.moveTo(totalSize / 2,totalSize / 2);
                //创建一个圆弧,占用矩形QRectF(x,y,宽度,高度),从指定的startAngle开始，并逆时针延伸angel度
                slicePath.arcTo(margin,margin,margin+pieSize,margin+pieSize,startAngle,angle);
                //绘画，关闭子路径启动新的路径
                slicePath.closeSubpath();
                //toFillPolygon将路径转换为多边形并返回。toPolygon通过将每个QPointF转换为QPoint创建并返回一个QPolygon,最终返回一个整数精度的点向量。
                return QRegion(slicePath.toFillPolygon().toPolygon());
            }
            startAngle += angle; //每查询一个数值就加角度
        }
    }
    return QRegion();
}

//返回给定索引的模型项的父项
int PieView::rows(const QModelIndex &index) const
{
    //parent返回具有给定索引的模型项的父项
    return model()->rowCount(model()->parent(index));
}

//设置滑动条
void PieView::updateGeometries()
{
    //保存页面步骤。视口宽度
    horizontalScrollBar()->setPageStep(viewport()->width());
    //设置滑块的最小值和最大值。
    horizontalScrollBar()->setRange(0,qMax(0,2 * totalSize - viewport()->width()));
    //垂直滑动块设置
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0,qMax(0,totalSize - viewport()->height()));
}
