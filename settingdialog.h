#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDialog>

class settingdialog : public QDialog
{
    Q_OBJECT
public:
    settingdialog(QWidget *parent = nullptr);

private slots:
    void handel_cancel();
    void handel_save();

private:
    QVBoxLayout *Mainlayout;
};

#endif // SETTINGDIALOG_H
