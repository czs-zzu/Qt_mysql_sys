#include "custom_show.h"
#include "ui_custom_show.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include "indent_show.h"
#include <QMessageBox>
#include "widget.h"

custom_show::custom_show(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::custom_show)
{
    ui->setupUi(this);
    show_cam_info();//在创建该页面时就更新相机表格中的数据
    show_indent_info();//更新订单界面的信息
    connect(ui->pushButton_place_order, &QPushButton::clicked, this, &custom_show::process_indent_info);
    connect(ui->pushButton_clearing, &QPushButton::clicked, this, &custom_show::process_clearing);

    //connect(ui->tab_indent, &QTableWidget::clicked, this, &custom_show::on_tableWidget_indent_clicked);
    //按下下单按钮，就出现indent_show界面
    //indent_show* s = new indent_show;
    //connect(ui->pushButton_place_order, SIGNAL(clicked()));
}

custom_show::~custom_show()
{
    delete ui;
}

void custom_show::process_clearing()
{
    QList<QTableWidgetItem*> items = ui->tableWidget_indent->selectedItems();
    if(items.size() == 0)
    {
        QMessageBox::about(this, "错误！", "你没有选中要结算的账单！");
        return;
    }
    //将总租金转换为float型。
    QTableWidgetItem* item = items.at(0);
    float final_charge = item->text().toFloat();
    qDebug() << final_charge;

    //获取当前用户的余额。
    QString command = tr("select balance from customs where user_name = \"%1\";").arg(Widget::user_name);
    QSqlQuery query(command);
    int nRow = query.size();
    if(nRow != 1)
    {
        //输出错误信息
        QMessageBox::about(this, "错误！", "没有查到当前用户的余额信息！");
        return;
    }
    //进行余额转换
    query.first();
    float balance = query.value(0).toFloat();
    qDebug() << "余额：" << balance;//输出一直为0，以为有错误结果是账户1里面就没钱，，，给他充点钱。
    if(final_charge < 0)
    {
        QMessageBox::about(this, "错误！", "老哥，账单为负。结账后你还赚钱呢，联系管理员吧，这个单结不了。");
        return;
    }
    //嘿嘿，扣钱了
    balance -= final_charge;
    if(balance < 0)
    {
        QMessageBox::about(this, "错误！", "老哥，你没钱了，找管理员充钱吧。充钱让你更强大。");
        return;
    }
    //将账户余额重新写入账户里。
    command = tr("update customs set balance = \"%1\";").arg(balance);
    query.exec(command);
}

//暂时不需要
//void custom_show::on_tableWidget_indent_clicked()
//{
//    //根据当前用户名，从indent表中获取相关记录。
//    QString current_user = Widget::user_name;
//    QString command = tr("select * from indent where user_name = \"%1\";").arg(current_user);
//    qDebug() << command;
//}

void custom_show::process_indent_info()
{
    //获取客户选中的相机信息
    //先默认一次只能下单一个相机吧。
    QList<QTableWidgetItem*> items = ui->tableWidget_browse->selectedItems();
    if(items.size() == 0)
    {
        QMessageBox::about(this, "错误！", "你没有选中相机编号！");
        return;
    }
    QTableWidgetItem* item = items.at(0);
    QString camera_no = item->text();//这里获得相机编号
    qDebug() << camera_no;
    //触发相机编号信号，好让indent_show界面接收到相机编号信息，用于其界面提交按钮发送命令到数据库。

    //显示订单界面，也可以说成跳转到订单界面，不过是非模式界面。
    indent_show* s = new indent_show();
    s->setCameraNo(camera_no);
    s->show();
    connect(s, SIGNAL(want_to_refresh_indent()), this, SLOT(show_indent_info()));
}


void custom_show::show_cam_info()
{
    //更新相机信息。
    QString command = "select * from cameras";
    QSqlQuery query(command);
    int nRow = query.size();
    QSqlRecord sqlRecord = query.record();
    int nColumn = sqlRecord.count();
    ui->tableWidget_browse->setColumnCount(nColumn);
    ui->tableWidget_browse->setRowCount(nRow);
    QStringList headers;
    headers << QStringLiteral("相机编号")<< QStringLiteral("相机型号")<< QStringLiteral("总量")<< QStringLiteral("已借出量")<< QStringLiteral("租金/天");
    ui->tableWidget_browse->setHorizontalHeaderLabels(headers);
    int i = 0;
    query.first();
    do{
        for (int j=0; j<nColumn; j++)
        {
            ui->tableWidget_browse->setItem(i, j, new QTableWidgetItem(query.value(j).toString()));
        }
        i++;
    }while(query.next());
    //更新完毕
}

void custom_show::show_indent_info()
{
    QString current_user_name = Widget::user_name;
    QString command = tr("select * from my_indent where user_name = \"%1\";").arg(current_user_name);
    qDebug() << command;
    QSqlQuery query(command);
    query.exec();
    int nRow = query.size();
    QSqlRecord sqlRecord = query.record();
    int nColumn = sqlRecord.count();
    ui->tableWidget_indent->setRowCount(nRow);
    ui->tableWidget_indent->setColumnCount(nColumn);
    QStringList headers;
    headers << QStringLiteral("订单号") << QStringLiteral("用户名") << QStringLiteral("相机编号") << QStringLiteral("租赁数量")
            << QStringLiteral("单价") << QStringLiteral("开始时间") << QStringLiteral("结束时间") << QStringLiteral("租赁天数")
            << QStringLiteral("租赁费用") << QStringLiteral("额外费用") << QStringLiteral("总费用");
    ui->tableWidget_indent->setHorizontalHeaderLabels(headers);
    int i=0;
    query.first();
    do{
        for(int j=0; j<nColumn; j++)
        {
            if(j==5 || j==6)//转换时间
            {
                QString str_time = query.value(j).toString();
                unsigned int int_time = str_time.toInt();
                QDateTime time = QDateTime::fromTime_t(int_time);
                ui->tableWidget_indent->setItem(i, j, new QTableWidgetItem(time.toString("yyyy/MM/dd")));//toString()对时间输出进行了格式化。
                continue;//跳过这次循环。
            }
            ui->tableWidget_indent->setItem(i, j, new QTableWidgetItem(query.value(j).toString()));
        }
        i++;
    }while(query.next());
//    int i=0;
//    query.first();
//    do{
//        for(int j=0; j<=nColumn; j++)//多出一列，用于计算应该支付的金额
//        {
//            //这一段写的挺差的
//            if(j==4 || j==5)//转换时间
//            {
//                QString str_time = query.value(j).toString();
//                unsigned int int_time = str_time.toInt();
//                QDateTime time = QDateTime::fromTime_t(int_time);
//                ui->tableWidget_indent->setItem(i, j, new QTableWidgetItem(time.toString("yyyy/MM/dd")));//toString()对时间输出进行了格式化。
//                continue;//跳过这次循环。
//            }
//            if(j == nColumn)//最后一格，应该支付的金额
//            {
//                //获取归还时间与开始时间，有字符串转为整型再转为QdateTime型，由daysTo()函数计算出相差的天数。
//                QString str_time = query.value(4).toString();
//                QDateTime time_start = QDateTime::fromTime_t(str_time.toInt());
//                str_time = query.value(5).toString();
//                QDateTime time_rent = QDateTime::fromTime_t(str_time.toInt());
//                qint64 rent_days = time_start.daysTo(time_rent);
//                //qDebug() << rent_days;
//                //根据相机编号找到他的单价，再计算出总价，我太难了。
//                QString camera_no = query.value(3).toString();
//                QString command_find_price = tr("select price from cameras where camera_no = \"%1\"").arg(camera_no);
//                qDebug() << command_find_price;
//                QSqlQuery query_find_price(command_find_price);//只能查找到一个，camera_no是唯一的。
//                qDebug() <<"query_find_price.value(0).toString():" << query_find_price.value(0).toString();
//                int price = query_find_price.value(0).toInt();
//                qDebug() << "单价为：" << price;
//                int charge = price * rent_days;
//                charge += query.value(6).toInt();
//                qDebug() << "额外费用为：" << query.value(6).toInt();
//                qDebug() << "总费用为：" <<charge;
//            }
//            ui->tableWidget_indent->setItem(i, j, new QTableWidgetItem(query.value(j).toString()));
//        }
//        i++;
//    }while(query.next());
}
