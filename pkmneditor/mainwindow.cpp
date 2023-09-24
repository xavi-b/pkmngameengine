#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("Pkmns Editor");

    createMenus();

    typesView = new TypesView;
    pkmnsView = new PkmnsView;

    QWidget*     w          = new QWidget;
    QVBoxLayout* vLayout    = new QVBoxLayout;
    QWidget*     gridWidget = new QWidget;
    QGridLayout* gridLayout = new QGridLayout;

    QPushButton* button1 = new QPushButton(tr("Types"));
    button1->setFixedSize(200, 150);
    connect(button1, &QPushButton::clicked, this, [=]() {
        stackedWidget->setCurrentWidget(typesView);
    });
    gridLayout->addWidget(button1, 0, 0);
    QPushButton* button2 = new QPushButton(tr("Abilities"));
    button2->setFixedSize(200, 150);
    gridLayout->addWidget(button2, 0, 1);
    QPushButton* button3 = new QPushButton(tr("Moves"));
    button3->setFixedSize(200, 150);
    gridLayout->addWidget(button3, 0, 2);
    QPushButton* button4 = new QPushButton(tr("Items"));
    button4->setFixedSize(200, 150);
    gridLayout->addWidget(button4, 1, 0);
    QPushButton* button5 = new QPushButton(tr("Pkmns"));
    button5->setFixedSize(200, 150);
    connect(button5, &QPushButton::clicked, this, [=]() {
        stackedWidget->setCurrentWidget(pkmnsView);
    });
    gridLayout->addWidget(button5, 1, 1);

    gridWidget->setLayout(gridLayout);
    vLayout->addWidget(gridWidget, 0, Qt::AlignCenter);
    w->setLayout(vLayout);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(w);

    connect(typesView, &TypesView::back, this, [=]() {
        stackedWidget->setCurrentIndex(0);
    });
    stackedWidget->addWidget(typesView);
    connect(pkmnsView, &PkmnsView::back, this, [=]() {
        stackedWidget->setCurrentIndex(0);
    });
    stackedWidget->addWidget(pkmnsView);

    setCentralWidget(stackedWidget);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openFolderAct = new QAction(tr("Open &folder"));
    connect(openFolderAct, &QAction::triggered, this, [=]() {
        QString lastDir = QSettings().value("lastDir").toString();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Open folder"), lastDir);

        if (dirName.isEmpty())
            return;
        else
        {
            openedFileName = "";

            // Types
            types = Type::vectorFromPropertyTree(readPropertyTree(dirName, "types.txt"));
            typesView->setTypes(types);
            // Pkmns
            pkmns = PkmnDef::vectorFromPropertyTree(readPropertyTree(dirName, "pokemon.txt"));
            pkmnsView->setPkmns(pkmns);

            stackedWidget->setCurrentIndex(0);
            QSettings().setValue("lastDir", dirName);
            QSettings().sync();
        }
    });
    QAction* openFileAct = new QAction(tr("&Open file"));
    openFileAct->setShortcuts(QKeySequence::Open);
    connect(openFileAct, &QAction::triggered, this, [=]() {
        QString lastDir  = QSettings().value("lastDir").toString();
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), lastDir, tr("JSON (*.json)"));

        if (fileName.isEmpty())
            return;
        else
        {
            QFile file(fileName);

            if (!file.open(QIODevice::ReadOnly))
            {
                QMessageBox::information(this, tr("Unable to open file"), file.errorString());
                return;
            }

            openedFileName  = fileName;
            QByteArray data = file.readAll();
            js::value  json = js::parse(data.data());

            // Types
            types = js::value_to<std::vector<Type::TypePtr>>(json.as_object()["types"]);
            typesView->setTypes(types);
            // Pkmns
            pkmns = js::value_to<std::vector<PkmnDef::PkmnDefPtr>>(json.as_object()["pkmns"]);
            pkmnsView->setPkmns(pkmns);

            stackedWidget->setCurrentIndex(0);
            QSettings().setValue("lastDir", QFileInfo(fileName).dir().path());
            QSettings().sync();
        }
    });
    QAction* saveAct = new QAction(tr("&Save"));
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    QAction* saveAsAct = new QAction(tr("Save &as"));
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAct, &QAction::triggered, this, [=]() {
        if (QString fileName = saveFile(true); !fileName.isEmpty())
            openedFileName = fileName;
    });
    QAction* spritesAct = new QAction(tr("Select sprites folder"));
    connect(spritesAct, &QAction::triggered, this, [=]() {
        QString lastDir = QSettings().value("lastSpritesDir").toString();
        QString dirName = QFileDialog::getExistingDirectory(this, tr("Open folder"), lastDir);

        if (dirName.isEmpty())
            return;
        else
        {
            pkmnsView->setSpritesDirectory(dirName);

            QSettings().setValue("lastSpritesDir", dirName);
            QSettings().sync();
        }
    });
    fileMenu->addAction(openFolderAct);
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(saveAct);
    fileMenu->addSeparator();
    fileMenu->addAction(spritesAct);
}

QString MainWindow::saveFile(bool saveAs)
{
    QString fileName = openedFileName;

    if (saveAs || openedFileName.isEmpty())
    {
        fileName =
            QFileDialog::getSaveFileName(this, tr("Save map"), saveAs ? "" : openedFileName, tr("JSON (*.json)"));
    }

    if (fileName.isEmpty())
        return {};
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return {};
        }

        js::object json;

        // Types
        json["types"] = js::value_from(types);
        // Pkmns
        json["pkmns"] = js::value_from(pkmns);

        file.write(js::serialize(json).c_str());
    }

    QSettings().setValue("lastDir", QFileInfo(fileName).dir().path());
    QSettings().sync();

    return fileName;
}

pt::ptree MainWindow::readPropertyTree(QString const& dirName, QString const& fileName)
{
    pt::ptree ptPkmns;

    std::ifstream pkmnsFile(QDir(dirName).absoluteFilePath(fileName).toStdString());
    if (pkmnsFile.is_open())
    {
        std::string       line;
        std::stringstream stream;

        while (getline(pkmnsFile, line))
        {
            if (line[0] != '#')
                stream << line << '\n';
        }
        pkmnsFile.close();

        pt::ini_parser::read_ini(stream, ptPkmns);
    }
    else
    {
        QMessageBox::warning(this, "Opening file issue", "Unable to open file for reading: " + fileName);
    }

    return ptPkmns;
}
