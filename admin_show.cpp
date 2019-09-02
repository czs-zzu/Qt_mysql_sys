#include "admin_show.h"
#include "ui_admin_show.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <qdebug.h>
#include <QMessageBox>

admin_show::admin_show(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::admin_show)
{
    ui->setupUi(this);
    setWindowTitle("管理员界面");
    show_admin_info();//显示管理员信息
    show_custom_info();//显示客户信息
    show_indent_info();//显示订单信息

    dialog_add_admin = new Dialog_add_admin (this);//先初始化指针，要不然下面connect函数调用时，程序会崩溃。
    dialog_edit_admin = new Dialog_edit_admin(this);
    dialog_find_admin = new Dialog_find_admin(this);
    dialog_add_custom = new Dialog_add_custom(this);
    dialog_edit_custom = new Dialog_edit_custom(this);
    dialog_find_custom = new Dialog_find_custom(this);
    dialog_edit_indent = new Dialog_edit_indent(this);

    connect(ui->pushButton_delete_admin, SIGNAL(clicked()), this, SLOT(delete_admin()));

    //接收到dialog_add_admin发送的want_to_refresh_admin_info()信号，就调用show_admin_info();
    connect(dialog_add_admin, &Dialog_add_admin::want_to_refresh_admin_info, this, &admin_show::show_admin_info);
    connect(dialog_edit_admin, &Dialog_edit_admin::want_to_refresh_admin_info, this, &admin_show::show_admin_info);
    connect(dialog_add_custom, &Dialog_add_custom::want_to_refresh_custom_info, this, &admin_show::show_custom_info);
    connect(dialog_edit_custom, &Dialog_edit_custom::want_to_refresh_custom, this, &admin_show::show_custom_info);
}

admin_show::~admin_show()
{
    delete ui;
}

void admin_show::delete_admin()
{
    int current_row = ui->tableWidget_admin->currentRow();//表格中选中的行数。
    if(current_row == -1)
    {
        QMessageBox::about(this, "错误", "没有选中任何一条记录！");
        return;
    }
    QString admin_name = ui->tableWidget_admin->item(current_row, 0)->text();
    QString command = tr("delete from admins where admin_name = \"%1\";").arg(admin_name);
    qDebug() << "command:" << command;
    QSqlQuery query(command);
    query.exec();

    show_admin_info();
}

void admin_show::show_custom_info()
{
    QString command = tr("select * from customs;");
    qDebug() <<"show_customs_info():" << command;
    QSqlQuery query(command);
    query.exec();
    int nRow = query.size();
    QSqlRecord sqlRecord = query.record();
    int nColumn = sqlRecord.count();
    ui->tableWidget_custom->setRowCount(nRow);
    ui->tableWidget_custom->setColumnCount(nColumn);
    int i=0;
    query.first();
    do{
        for(int j=0; j<nColumn; j++)
        {
            ui->tableWidget_custom->setItem(i, j, new QTableWidgetItem(query.value(j).toString()));
        }
        i++;
    }while(query.next());
}

void admin_show::show_indent_info()
{
    QString command = tr("select * from my_indent;");
    QSqlQuery query(command);
    query.exec();
    int nRow = query.size();
    QSqlRecord sqlRecord = query.record();
    int nColumn = sqlRecord.count();
    ui->tableWidget_indent->setRowCount(nRow);
    ui->tableWidget_indent->setColumnCount(nColumn);
    int i=0;
    query.first();
    do{
        for(int j=0; j<nColumn; j++)
        {
            ui->tableWidget_indent->setItem(i, j, new QTableWidgetItem(query.value(j).toString()));
        }
        i++;
    }while(query.next());
}

void admin_show::show_admin_info()
{
    QString command = tr("select * from admins;");
    qDebug() <<"show_admin_info():" << command;
    QSqlQuery query(command);
    query.exec();
    int nRow = query.size();
    QSqlRecord sqlRecord = query.record();
    int nColumn = sqlRecord.count();
    ui->tableWidget_admin->setRowCount(nRow);
    ui->tableWidget_admin->setColumnCount(nColumn);
    int i=0;
    query.first();
    do{
        for(int j=0; j<nColumn; j++)
        {
            ui->tableWidget_admin->setItem(i, j, new QTableWidgetItem(query.value(j).toString()));
        }
        i++;
    }while(query.next());
}

//添加管理员按钮被点击事件
//显示对话框
void admin_show::on_pushButton_add_admin_clicked()
{

    dialog_add_admin->setModal(true);
    dialog_add_admin->show();
}

void admin_show::on_pushButton_alter_admin_clicked()
{
    //弹出界面，设置初值，修改，写回。
    //为dialog_edit_admin设置初值
    //获取账户密码
    int current_row = ui->tableWidget_admin->currentRow();//表格中选中的行数。
    if(current_row == -1)
    {
        QMessageBox::about(this, "错误", "没有选中任何一条记录！");
        return;
    }
    QString account = ui->tableWidget_admin->item(current_row, 0)->text();
    QString password = ui->tableWidget_admin->item(current_row, 1)->text();

    dialog_edit_admin->setAccount(account);
    dialog_edit_admin->setPassword(password);
    dialog_edit_admin->setModal(true);
    dialog_edit_admin->show();
}


//查找按钮被点击后
void admin_show::on_pushButton_find_admin_clicked()
{
    dialog_find_admin->setModal(true);
    dialog_find_admin->show();
}

void admin_show::on_pushButton_add_custom_clicked()
{
    dialog_add_custom->setModal(true);
    dialog_add_custom->show();
}

//删除用户
void admin_show::on_pushButton_delete_custom_clicked()
{
    int current_row = ui->tableWidget_custom->currentRow();//表格中选中的行数。
    if(current_row == -1)
    {
        QMessageBox::about(this, "错误", "没有选中任何一条记录！");
        return;
    }
    QString custom_name = ui->tableWidget_custom->item(current_row, 0)->text();
    QString command = tr("delete from customs where user_name = \"%1\";").arg(custom_name);
    qDebug() << "command:" << command;
    QSqlQuery query(command);
    query.exec();

    show_custom_info();
}

void admin_show::on_pushButton_alter_custom_clicked()
{
    int current_row = ui->tableWidget_custom->currentRow();//表格中选中的行数。
    //qDebug() << "current_row:" << current_row;
    if(current_row == -1)
    {
        QMessageBox::about(this, "错误", "没有选中任何一条记录！");
        return;
    }
    QString account = ui->tableWidget_custom->item(current_row, 0)->text();
    QString password = ui->tableWidget_custom->item(current_row, 1)->text();
    double balance = ui->tableWidget_custom->item(current_row, 2)->text().toDouble();
    QString sex = ui->tableWidget_custom->item(current_row, 3)->text();
    QString phone = ui->tableWidget_custom->item(current_row, 4)->text();

    dialog_edit_custom->setAllInfo(account, password, balance, sex, phone);
    dialog_edit_custom->setModal(true);
    dialog_edit_custom->show();
}

void admin_show::on_pushButton_find_custom_clicked()
{
    dialog_find_custom->setModal(true);
    dialog_find_custom->show();
}

void admin_show::on_pushButton_alter_indent_clicked()
{
    int current_row = ui->tableWidget_indent->currentRow();//表格中选中的行数。
    if(current_row == -1)
    {
        QMessageBox::about(this, "错误", "没有选中任何一条记录！");
        return;
    }
    QString order_no = ui->tableWidget_indent->item(current_row, 0)->text();
    QString account = ui->tableWidget_indent->item(current_row, 1)->text();
    QString camera_no = ui->tableWidget_indent->item(current_row, 2)->text();
    int rent_num = ui->tableWidget_indent->item(current_row, 3)->text().toInt();
    int price = ui->tableWidget_indent->item(current_row, 4)->text().toInt();
    unsigned int start_time = ui->tableWidget_indent->item(current_row, 5)->text().toInt();
    unsigned int rent_time = ui->tableWidget_indent->item(current_row, 6)->text().toInt();
    double rent_days = ui->tableWidget_indent->item(current_row, 7)->text().toDouble();

    double rent_charge = ui->tableWidget_indent->item(current_row, 8)->text().toDouble();
    double extra_charge = ui->tableWidget_indent->item(current_row, 9)->text().toDouble();
    double final_charge = ui->tableWidget_indent->item(current_row, 10)->text().toDouble();
//    QString rent_charge = ui->tableWidget_indent->item(current_row, 0)->text();
//    QString extra_charge = ui->tableWidget_indent->item(current_row, 0)->text();
//    QString final_charge = ui->tableWidget_indent->item(current_row, 0)->text();
    qDebug() << "rent_charge:" << rent_charge << extra_charge << final_charge;

    dialog_edit_indent->setModal(true);
    dialog_edit_indent->setAllInfo(order_no, account, camera_no, rent_num, price, start_time, rent_time, rent_days, rent_charge, extra_charge, final_charge);
    dialog_edit_indent->show();
}
