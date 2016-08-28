#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPixmap>
#include "programs.h"


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:

    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void hiding();

public slots:

    void updateProgramList();
    void initMenu();
    void addProgram();
    void setStartWithWindows(bool start);
    void addToContextMenu(bool contextmenu);

protected:
    virtual void dropEvent(QDropEvent* event);

    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

    bool alreadyExists(QString fileName);

      QAction * startWithWindowsAction;
      QAction * addToContextMenuAction;

      QAction * aboutAction;

      void initAutostart();
      void initContextMenu();

private slots:

    void on_toolButton_clicked();

    void on_Del_clicked();

    void on_FilePath_clicked();

    void on_startProgram_clicked();

    void on_unblockButton_clicked();

    void on_blockButton_clicked();

    void on_aboutAction();

private:
    Ui::Widget *ui;
    Programs programs;
};

#endif // WIDGET_H
