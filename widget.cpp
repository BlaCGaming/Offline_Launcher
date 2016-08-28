#include <QCloseEvent>
#include "widget.h"
#include "config.h"
#include "aboutdialog.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QListWidget>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>



Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)

{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);

    programs.load();
    updateProgramList();
    initMenu();

    setAcceptDrops(true);
}

Widget::~Widget()
{
    programs.save();
    delete ui;
}

void Widget::updateProgramList()
{
    ui->listWidget->clear();
    for(Program *program: programs.getPrograms()->toStdList())
    {

        QString fileName = program->getFileName();
        QListWidgetItem * widgetItem = new QListWidgetItem(fileName);
        widgetItem->setIcon(program->getIcon());
        QColor color;
        switch(program->getStatus())
        {
        case Program::AlwaysActive:

            widgetItem->setTextColor(color.green());
            break;
        case Program::SessionActive:
            widgetItem->setTextColor(color.black());
            break;
        case Program::Deactivated:
            widgetItem->setTextColor(color.red());
            break;
        default:
            widgetItem->setTextColor(color.black());
        }

        ui->listWidget->addItem(widgetItem);
    }
}

void Widget::closeEvent(QCloseEvent * event)
{
    event->ignore();
    this->hide();
    emit hiding();
}

bool Widget::alreadyExists(QString fileName)
{
    for(int i = 0; i < ui->listWidget->count(); ++i)
    {
        if(ui->listWidget->item(i)->text() == fileName)
            return true;
    }
    return false;
}

void Widget::on_toolButton_clicked()
{

    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open File"), Config::instance()->lastPath(),
                                                          tr("Executables (*.exe);; All Files (*.*)"));

    ui->progressBar->setMaximum(fileNames.count());
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    for(QString fileName: fileNames){

        ui->progressBar->setValue(ui->progressBar->value() + 1);

        if (!fileName.isNull())
        {
            Config::instance()->setLastPath(QFileInfo(fileName).path()); // store path for next time
        }
        if (fileName.isEmpty())
            return;

        if(alreadyExists(fileName)){
            QMessageBox::information(this, "Already Exists", fileName + " already exists.");
            continue; // insert next item, not this one !
        }

        programs.addProgram(fileName);

        updateProgramList();
        
    }
    ui->progressBar->setVisible(false);
}

void Widget::dropEvent(QDropEvent* event) {
    // Drop part
    if (event && event->mimeData()) {
        const QMimeData *mData = event->mimeData();
        // Drop Images from FileManager into ImageList
        if (mData->hasUrls()) {
            QList<QUrl> urls = mData->urls();
            QStringList files;
            for (int x = 0; x < urls.count(); ++x) {
                if (QFileInfo(urls.at(x).toLocalFile()).isDir()){
                }
                else
                {
                    QStringList fileNames;
                    fileNames = QStringList(urls.at(x).toLocalFile());


                }
            }
        }
    }
}

void Widget::keyPressEvent(QKeyEvent * event)
{
    if (event->key()== Qt::Key_Delete)
    {
    on_Del_clicked();
    }
}

void Widget::on_Del_clicked()
{

    for(QListWidgetItem *item: ui->listWidget->selectedItems()){
        programs.deleteProgram(ui->listWidget->row(item));
        ui->listWidget->removeItemWidget(item);
        delete item;
    }
    updateProgramList();

}
void Widget::on_FilePath_clicked()
{
    if(ui->listWidget->currentItem() != NULL)
    {
        QString fileName = ui->listWidget->currentItem()->text();

        QStringList args;

        args << "/select," << QDir::toNativeSeparators(fileName);

        QProcess::startDetached("explorer", args);
    }
    else
    {
        QMessageBox::warning(this, "", "Bitte wählen sie einen Eintrag aus der Liste");
    }
}

void Widget::initMenu()
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *settings = new QMenu(tr("&Settings"), this);
    QMenu *help = new QMenu(tr("&Help"), this);
    help->addSeparator();
    menuBar->addMenu(settings);
    menuBar->addMenu(help);


    help->setStyleSheet("QMenu::item:selected {background-color: rgb(51, 153, 255);}");
    settings->setStyleSheet("QMenu::item:selected {background-color: rgb(51, 153, 255);}");

    startWithWindowsAction = settings->addAction("Start with Windows");
    addToContextMenuAction = settings->addAction("Add to Context Menu");
    addToContextMenuAction->setToolTip("This function is only available with admin privileges");

    initAutostart();
    initContextMenu();

    help->addAction("Version");
    help->addAction("Check for Update");
    help->addAction("Donate");
    aboutAction = help->addAction("About");
    help->addAction("Submit Feedback");

    this->layout()->setMenuBar(menuBar);
    connect(startWithWindowsAction, SIGNAL(toggled(bool)), this, SLOT(setStartWithWindows(bool)));
    connect(addToContextMenuAction, SIGNAL(toggled(bool)), this, SLOT(addToContextMenu(bool)));
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(on_aboutAction()));

}

void Widget::initAutostart(){

    startWithWindowsAction->setCheckable(true);

#ifdef _WIN32
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if(registry.childKeys().contains("Offline Launcher"))
        startWithWindowsAction->setChecked(true);
#endif
}

void Widget::initContextMenu()
{

    addToContextMenuAction->setCheckable(true);

#ifdef _WIN32
    QSettings registry("HKEY_CLASSES_ROOT\\.exe\\shell", QSettings::NativeFormat);
    if(registry.contains("OfflineLauncher"))
        addToContextMenuAction->setChecked(true);
#endif
}

void Widget::addProgram()
{
    show();
    on_toolButton_clicked();
}

void Widget::setStartWithWindows(bool start)
{
#ifdef _WIN32
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (start)
    {
        registry.setValue("Offline Launcher", "\"" + QCoreApplication::applicationFilePath().replace("/", "\\") + "\" --autostart");
    }else {
        registry.remove("Offline Launcher");
    }
#endif
}

void Widget::addToContextMenu(bool contextmenu)
{
#ifdef _WIN32

    if (contextmenu)
    {
        QSettings registry("HKEY_CLASSES_ROOT\\exefile\\shell\\OfflineLauncher\\command", QSettings::NativeFormat);

        registry.setValue("Default", "\"" + QCoreApplication::applicationFilePath().replace("/", "\\") + "\" \" %1\"");

        if(registry.status() == QSettings::AccessError){
            QMessageBox::warning(this, "Registry not Writable", "Please restart with admin rights to change this setting.");
            disconnect(addToContextMenuAction, SIGNAL(toggled(bool)), this, SLOT(addToContextMenu(bool)));
            addToContextMenuAction->setChecked(false);
            addToContextMenuAction->setEnabled(false);

        }

    }else {
        QSettings registry("HKEY_CLASSES_ROOT\\exefile\\shell", QSettings::NativeFormat);
        registry.setValue("OfflineLauncher\\command\\Default", "");
        registry.remove("OfflineLauncher");

        if(registry.status() == QSettings::AccessError){
            QMessageBox::information(this, "Registry not Writable", "Please restart with admin rights to change this setting.");
            disconnect(addToContextMenuAction, SIGNAL(toggled(bool)), this, SLOT(addToContextMenu(bool)));
            addToContextMenuAction->setChecked(true);
            addToContextMenuAction->setEnabled(false);
        }
    }
#endif
}


void Widget::on_startProgram_clicked()
{

    for(QListWidgetItem *item: ui->listWidget->selectedItems()){

        QString program = item->text();
        Program * prog = programs.getProgram(program);
        if(prog != 0)
                prog->start();
    }
}

void Widget::on_unblockButton_clicked()
{
    for(QListWidgetItem *item: ui->listWidget->selectedItems()){

        QString program = item->text();
        Program * prog = programs.getProgram(program);
        if(prog != 0)
        {
            prog->unblock();
        }
    }
}

void Widget::on_blockButton_clicked()
{
    for(QListWidgetItem *item: ui->listWidget->selectedItems()){

        QString program = item->text();
        Program * prog = programs.getProgram(program);
        if(prog != 0)
        {
            prog->block();
        }
    }
}

void Widget::on_aboutAction()
{
    AboutDialog aboutDialog;
    aboutDialog.exec();
}
