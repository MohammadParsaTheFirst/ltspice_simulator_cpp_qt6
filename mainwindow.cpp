#include "mainwindow.h"
#include <QDir>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

#include <QDebug>
#include <QFileDialog>

QString getSubcircuitLibraryPath() {
    QString appPath = QCoreApplication::applicationDirPath();
    QDir dir(appPath + "/lib");
    if (!dir.exists())
        dir.mkpath(".");
    return dir.path();
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Initialize network manager
    networkManager = new NetworkManager(&circuit, this);
    connect(networkManager, &NetworkManager::connectionStatusChanged,
            this, &MainWindow::onNetworkStatusChanged);
    connect(networkManager, &NetworkManager::voltageSourceReceived,
            this, &MainWindow::onVoltageSourceReceived);
    connect(networkManager, &NetworkManager::circuitFileReceived,
            this, &MainWindow::onCircuitFileReceived);
    connect(networkManager, &NetworkManager::signalDataReceived,
            this, &MainWindow::onSignalDataReceived);
    connect(sendAction, &QAction::triggered, this, &MainWindow::hSendData); //added

    connect(networkManager, &NetworkManager::dataReceived, this, &MainWindow::onDataReceived); // Add this line

    loadSubcircuitsFromLibrary();
    setWindowIcon(QIcon(":/icon.png"));
    this->resize(900, 600);
    schematicsPath = QCoreApplication::applicationDirPath() + "/Schematics";
    QDir dir(schematicsPath);
    if (!dir.exists())
        dir.mkpath(".");

    starterWindow();
    setupWelcomeState();
}

void MainWindow::loadSubcircuitsFromLibrary() {
    QString libraryPath = getSubcircuitLibraryPath();
    QDir directory(libraryPath);
    QStringList files = directory.entryList(QStringList() << "*.sub", QDir::Files);

    for (const QString& filename : files) {
        QString filePath = libraryPath + "/" + filename;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("Could not open subcircuit file: %s", qPrintable(filePath));
            continue;
        }

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_6_5);
        SubcircuitDefinition subDef;
        in >> subDef;
        file.close();

        circuit.subcircuitDefinitions[subDef.name] = subDef;
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupWelcomeState() {
    this->setWindowTitle("ParsaSpice Simulator");

    if (centralWidget()) {
        delete centralWidget();
        schematic = nullptr;
    }

    QLabel* backgroundLabel = new QLabel(this);
    QPixmap backgroundImage(":/background.jpg");
    backgroundLabel->setPixmap(backgroundImage);
    backgroundLabel->setScaledContents(true);
    setCentralWidget(backgroundLabel);

    saveAction->setEnabled(false);
    configureAnalysisAction->setEnabled(false);
    runAction->setEnabled(false);
    wireAction->setEnabled(false);
    groundAction->setEnabled(false);
    voltageSourceAction->setEnabled(false);
    resistorAction->setEnabled(false);
    capacitorAction->setEnabled(false);
    inductorAction->setEnabled(false);
    diodeAction->setEnabled(false);
    nodeLibraryAction->setEnabled(false);
    labelAction->setEnabled(false);
    deleteModeAction->setEnabled(false);
    createSubcircuitAction->setEnabled(false);
    subcircuitLibraryAction->setEnabled(false);
}

void MainWindow::setupSchematicState(const QString& projectName) {
    if (centralWidget() && schematic) {
        delete centralWidget();
        schematic = nullptr;
    }

    this->setWindowTitle(projectName);
    schematic = new SchematicWidget(&circuit, this);
    setCentralWidget(schematic);

    connect(saveAction, &QAction::triggered, this, &MainWindow::hSaveProject);
    connect(runAction, &QAction::triggered, schematic, &SchematicWidget::startRunAnalysis);
    connect(configureAnalysisAction, &QAction::triggered, schematic, &SchematicWidget::startOpenConfigureAnalysis);
    connect(wireAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingWire);
    connect(groundAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingGround);
    connect(resistorAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingResistor);
    connect(capacitorAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingCapacitor);
    connect(inductorAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingInductor);
    connect(voltageSourceAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingVoltageSource);
    connect(diodeAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingDiode);
    connect(nodeLibraryAction, &QAction::triggered, schematic, &SchematicWidget::startOpenNodeLibrary);
    connect(labelAction, &QAction::triggered, schematic, &SchematicWidget::startPlacingLabel);
    connect(deleteModeAction, &QAction::triggered, schematic, &SchematicWidget::startDeleteComponent);
    connect(createSubcircuitAction, &QAction::triggered, schematic, &SchematicWidget::startCreateSubcircuit);
    connect(subcircuitLibraryAction, &QAction::triggered, schematic, &SchematicWidget::startOpeningSubcircuitLibrary);

    saveAction->setEnabled(true);
    configureAnalysisAction->setEnabled(true);
    runAction->setEnabled(true);
    wireAction->setEnabled(true);
    groundAction->setEnabled(true);
    voltageSourceAction->setEnabled(true);
    resistorAction->setEnabled(true);
    capacitorAction->setEnabled(true);
    inductorAction->setEnabled(true);
    diodeAction->setEnabled(true);
    nodeLibraryAction->setEnabled(true);
    labelAction->setEnabled(true);
    deleteModeAction->setEnabled(true);
    createSubcircuitAction->setEnabled(true);
    subcircuitLibraryAction->setEnabled(true);
}

void MainWindow::hShowSettings() {
    QMessageBox::information(this, "Settings", "Buy premium!");
}

void MainWindow::hNewSchematic() {
    bool ok;
    QString projectName = QInputDialog::getText(this, "New Project", "Enter project name:", QLineEdit::Normal, "", &ok);
    if (ok && !projectName.isEmpty()) {
        circuit.clearSchematic();
        currentProjectPath.clear();
        currentProjectName = projectName;
        setupSchematicState("ParsaSpice - " + projectName);
    }
}

void MainWindow::hSaveProject() {
    QString filePath = currentProjectPath;
    if (filePath.isEmpty()) {
        QString projectFolderPath = schematicsPath + "/" + currentProjectName;
        QDir().mkpath(projectFolderPath);
        QString defaultPath = projectFolderPath + "/" + currentProjectName + ".psp";
        filePath = QFileDialog::getSaveFileName(this, "Save Schematic", defaultPath, "ParsaSpice Project (*.psp)");
        if (filePath.isEmpty())
            return;
        currentProjectPath = filePath;
    }

    try {
        circuit.saveToFile(currentProjectPath);
        QFileInfo fileInfo(currentProjectPath);
        setWindowTitle("ParsaSpice - " + fileInfo.fileName());
        QMessageBox::information(this, "Success", "Project saved successfully.");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to save project: %1").arg(e.what()));
    }
}

void MainWindow::hOpenProject() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Schematic", schematicsPath, "ParsaSpice Project (*.psp)");
    if (filePath.isEmpty())
        return;

    try {
        circuit.loadFromFile(filePath);
        currentProjectPath = filePath;
        QFileInfo fileInfo(filePath);
        currentProjectName = fileInfo.baseName();
        setupSchematicState("ParsaSpice - " + fileInfo.fileName());
        schematic->update();
        QMessageBox::information(this, "Success", "Project loaded successfully.");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load project: %1").arg(e.what()));
        circuit.clearSchematic();
        setupWelcomeState();
    }
}

void MainWindow::starterWindow() {
    initializeActions();

    connect(newSchematicAction, &QAction::triggered, this, &MainWindow::hNewSchematic);
    connect(openAction, &QAction::triggered, this, &MainWindow::hOpenProject);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::hShowSettings);
    connect(networkAction, &QAction::triggered, this, &MainWindow::hNetworkConnection);  // Add this connection

    shortcutRunner();
    implementMenuBar();
    implementToolBar();
}

void MainWindow::initializeActions() {
    settingsAction = new QAction(QIcon(":/icon/icons/settings.png"), "Settings", this);
    newSchematicAction = new QAction(QIcon(":/icon/icons/newSchematic.png"), "New Schematic (CTRL+N)", this);
    saveAction = new QAction(QIcon(":/icon/icons/save.png"), "Save (CTRL+S)", this);
    openAction = new QAction(QIcon(":/icon/icons/open.png"), "Open (CTRL+O)", this);
    configureAnalysisAction = new QAction(QIcon(":/icon/icons/configureAnalysis.png"), "Configure Analysis (A)", this);
    runAction = new QAction(QIcon(":/icon/icons/run.png"), "Run (ALT+R)", this);
    wireAction = new QAction(QIcon(":/icon/icons/wire.png"), "Wire (W)", this);
    groundAction = new QAction(QIcon(":/icon/icons/ground.png"), "Ground (G)", this);
    voltageSourceAction = new QAction(QIcon(":/icon/icons/voltageSource.png"), "Voltage Source (V)", this);
    resistorAction = new QAction(QIcon(":/icon/icons/resistor.png"), "Resistor (R)", this);
    capacitorAction = new QAction(QIcon(":/icon/icons/Capacitor.png"), "Capacitor (C)", this);
    inductorAction = new QAction(QIcon(":/icon/icons/inductor.png"), "Inductor (L)", this);
    diodeAction = new QAction(QIcon(":/icon/icons/diode.png"), "Diode (D)", this);
    nodeLibraryAction = new QAction(QIcon(":/icon/icons/nodeLibrary.png"), "Node Library (P)", this);
    labelAction = new QAction(QIcon(":/icon/icons/text.png"), "Text (T)", this);
    deleteModeAction = new QAction(QIcon(":/icon/icons/deleteMode.png"), "Delete Mode (Backspace or Del)", this);
    createSubcircuitAction = new QAction("Create Subcircuit", this);
    subcircuitLibraryAction = new QAction("Open Subcircuit Library", this);
    quitAction = new QAction("Exit", this);
    networkAction = new QAction(QIcon(":/icon/icons/network.png"), "Network", this);  // Add this
    sendAction = new QAction(QIcon(":/icon/icons/send.png"), "Send", this); // added
}

void MainWindow::implementMenuBar() {
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(newSchematicAction);
    file->addAction(openAction);
    file->addAction(saveAction);
    file->addSeparator();
    file->addAction(quitAction);

    QMenu* edit = menuBar()->addMenu(tr("&Edit"));
    edit->addAction(labelAction);
    edit->addAction(configureAnalysisAction);
    edit->addAction(resistorAction);
    edit->addAction(capacitorAction);
    edit->addAction(inductorAction);
    edit->addAction(diodeAction);
    edit->addAction(nodeLibraryAction);
    edit->addAction(wireAction);
    edit->addAction(groundAction);
    edit->addAction(deleteModeAction);
    edit->addAction(createSubcircuitAction);

    QMenu* hierarchy = menuBar()->addMenu(tr("&Hierarchy"));
    hierarchy->addAction(createSubcircuitAction);
    hierarchy->addAction(subcircuitLibraryAction);

    QMenu* view = menuBar()->addMenu(tr("&View"));

    QMenu* simulate = menuBar()->addMenu(tr("&Simulate"));
    simulate->addAction(runAction);
    simulate->addSeparator();
    simulate->addAction(settingsAction);
    simulate->addSeparator();
    simulate->addAction(configureAnalysisAction);

    QMenu* tools = menuBar()->addMenu(tr("&Tools"));
    tools->addAction(settingsAction);
    tools->addAction(networkAction);  // Add network action to tools menu

    QMenu* window = menuBar()->addMenu(tr("&Window"));

    QMenu* help = menuBar()->addMenu("&Help");
    help->addAction("About the program");
}

void MainWindow::implementToolBar() {
    QToolBar* mainToolBar = addToolBar("Main Toolbar");
    mainToolBar->setMovable(false);

    mainToolBar->addAction(settingsAction);
    mainToolBar->addAction(newSchematicAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addAction(configureAnalysisAction);
    mainToolBar->addAction(runAction);
    mainToolBar->addAction(wireAction);
    mainToolBar->addAction(groundAction);
    mainToolBar->addAction(voltageSourceAction);
    mainToolBar->addAction(resistorAction);
    mainToolBar->addAction(capacitorAction);
    mainToolBar->addAction(inductorAction);
    mainToolBar->addAction(diodeAction);
    mainToolBar->addAction(nodeLibraryAction);
    mainToolBar->addAction(labelAction);
    mainToolBar->addAction(deleteModeAction);
    mainToolBar->addAction(networkAction);  // Add network action to toolbar
    mainToolBar->addAction(networkAction); //added
    mainToolBar->addAction(sendAction); // added

    mainToolBar->setIconSize(QSize(40, 40));
}

void MainWindow::shortcutRunner() {
    newSchematicAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    configureAnalysisAction->setShortcut(QKeySequence(Qt::Key_A));
    runAction->setShortcut(QKeySequence(Qt::ALT | Qt::Key_R));
    wireAction->setShortcut(QKeySequence(Qt::Key_W));
    groundAction->setShortcut(QKeySequence(Qt::Key_G));
    voltageSourceAction->setShortcut(QKeySequence(Qt::Key_V));
    resistorAction->setShortcut(QKeySequence(Qt::Key_R));
    capacitorAction->setShortcut(QKeySequence(Qt::Key_C));
    inductorAction->setShortcut(QKeySequence(Qt::Key_L));
    diodeAction->setShortcut(QKeySequence(Qt::Key_D));
    nodeLibraryAction->setShortcut(QKeySequence(Qt::Key_P));
    labelAction->setShortcut(QKeySequence(Qt::Key_T));
    deleteModeAction->setShortcuts({QKeySequence(Qt::Key_Backspace), QKeySequence(Qt::Key_Delete)});
    networkAction->setShortcut(QKeySequence(Qt::Key_N));  // Add network shortcut
    sendAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z)); // added for sending
}


void MainWindow::hNetworkConnection() {
    NetworkDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            if (dialog.isServer()) {
                quint16 port = dialog.getPort();
                if (networkManager->startServer(port)) {
                    statusBar()->showMessage("Server started on port " + QString::number(port));
                } else {
                    QMessageBox::warning(this, "Server Error",
                                        "Failed to start server. Check if port is available.");
                }
            } else {
                QString host = dialog.getHost();
                quint16 port = dialog.getPort();

                if (networkManager->connectToServer(host, port)) {
                    statusBar()->showMessage("Connecting to " + host + ":" + QString::number(port));
                } else {
                    QMessageBox::warning(this, "Connection Error",
                                        "Failed to connect to server. Check host/port and try again.");
                }
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Network Error",
                                 QString("Network operation failed: %1").arg(e.what()));
        }
    }
}
// void MainWindow::hNetworkConnection() {
//     NetworkDialog dialog(this);
//     if (dialog.exec() == QDialog::Accepted) {
//         if (dialog.isServer()) {
//             if (networkManager->startServer(dialog.getPort())) {
//                 statusBar()->showMessage("Server started on port " + QString::number(dialog.getPort()));
//             }
//         } else {
//             if (networkManager->connectToServer(dialog.getHost(), dialog.getPort())) {
//                 statusBar()->showMessage("Connecting to " + dialog.getHost() + ":" + QString::number(dialog.getPort()));
//             }
//         }
//     }
// }

void MainWindow::onNetworkStatusChanged(bool connected, const QString& message) {
    statusBar()->showMessage(message);
    networkAction->setIcon(connected ? QIcon(":/icon/icons/network_connected.png") : QIcon(":/icon/icons/network.png"));
}

// void MainWindow::onVoltageSourceReceived(const QString& name, const QString& node1, const QString& node2,
//                                        double value, bool isSinusoidal, const std::vector<double>& sinParams) {
//     if (schematic) {
//         // Add the received voltage source to the circuit
//         circuit.addComponent("V", name.toStdString(), node1.toStdString(), node2.toStdString(),
//                            value, sinParams, {}, isSinusoidal);
//         schematic->update();
//         statusBar()->showMessage("Received voltage source: " + name);
//     }
// }
void MainWindow::onVoltageSourceReceived(const QString& name, const QString& node1, const QString& node2,
                                       double value, bool isSinusoidal,
                                       double offset, double amplitude, double frequency) {
    if (schematic) {
        // Add the received voltage source to the circuit
        std::vector<double> sinParams;
        if (isSinusoidal) {
            sinParams = {offset, amplitude, frequency};
        }
        circuit.addComponent("V", name.toStdString(), node1.toStdString(), node2.toStdString(),
                           value, sinParams, {}, isSinusoidal);
        schematic->update();
        statusBar()->showMessage("Received voltage source: " + name);
    }
}

void MainWindow::onCircuitFileReceived() {
    if (schematic) {
        schematic->update();
        statusBar()->showMessage("Circuit file received and loaded");
    }
}

void MainWindow::onSignalDataReceived(const std::map<double, double>& data, const QString& signalName) {
    // Create a plot window to display the received signal
    PlotTransientData* plotWindow = new PlotTransientData(this);
    plotWindow->addSeries(data, signalName);
    plotWindow->show();
    statusBar()->showMessage("Signal data received: " + signalName);
}



////////////////

void MainWindow::saveProject() {
    // Implementation that saves user actions to project.log.txt
    QFile file("project.log.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        // Write all user actions to the file
        // This should match your existing logging mechanism
        file.close();
    }
}

// Replace the entire hSendData method in mainwindow.cpp with this corrected version:
void MainWindow::hSendData() {
    qDebug() << "Send button clicked";

    // Check if connected
    if (!networkManager->isConnected()) {
        QMessageBox::warning(this, "Error", "You are not connected to a server or client.");
        return;
    }

    qDebug() << "Network is connected, showing send dialog";

    // Create send options dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Send Data");
    dialog.setMinimumWidth(350);
    dialog.setMinimumHeight(200);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* label = new QLabel("What do you want to send?", &dialog);
    label->setStyleSheet("font-weight: bold; font-size: 12px;");

    QComboBox* comboBox = new QComboBox(&dialog);
    comboBox->addItem("Send the whole circuit file");
    comboBox->addItem("Send a signal file");
    comboBox->addItem("Send voltage source to node");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    layout->addWidget(label);
    layout->addWidget(comboBox);
    layout->addWidget(buttonBox);
    layout->setSpacing(15);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Show the dialog modally
    if (dialog.exec() != QDialog::Accepted) {
        qDebug() << "Send dialog cancelled";
        return;
    }

    QString selectedOption = comboBox->currentText();
    qDebug() << "Selected option:" << selectedOption;

    QByteArray dataToSend;
    QString prefix;

    try {
        if (selectedOption == "Send the whole circuit file") {
            // Ensure we have a current project
            if (currentProjectPath.isEmpty()) {
                QMessageBox::warning(this, "Error", "No circuit project is currently open.");
                return;
            }

            // Load the circuit file content
            QFile file(currentProjectPath);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Could not open circuit file: " + currentProjectPath);
                return;
            }

            dataToSend = file.readAll();
            file.close();
            prefix = "CIRCUIT:";

            qDebug() << "Circuit file loaded, size:" << dataToSend.size() << "bytes";
        }
        else if (selectedOption == "Send a signal file") {
            QString fileName = QFileDialog::getOpenFileName(
                this,
                "Select Signal File",
                QCoreApplication::applicationDirPath(),
                "Text Files (*.txt);;All Files (*)"
            );

            if (fileName.isEmpty()) {
                qDebug() << "No signal file selected";
                return;
            }

            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Error", "Could not open signal file: " + fileName);
                return;
            }

            dataToSend = file.readAll();
            file.close();
            prefix = "SIGNAL:";

            qDebug() << "Signal file loaded, size:" << dataToSend.size() << "bytes";
        }
        else if (selectedOption == "Send voltage source to node") {
            bool ok;
            QString nodeName = QInputDialog::getText(
                this,
                "Node Name",
                "Enter node name:",
                QLineEdit::Normal,
                "N1",
                &ok
            );

            if (!ok || nodeName.isEmpty()) {
                qDebug() << "No node name entered";
                return;
            }

            double voltage = QInputDialog::getDouble(
                this,
                "Voltage Value",
                "Enter voltage value:",
                0, -1000, 1000, 2, &ok
            );

            if (!ok) {
                qDebug() << "No voltage value entered";
                return;
            }

            QString message = QString("VOLTAGE %1 %2").arg(nodeName).arg(voltage);
            dataToSend = message.toUtf8();
            prefix = "VOLTAGE:";

            qDebug() << "Voltage data prepared:" << message;
        }

        // Prepend the prefix and send the data
        QByteArray finalData = prefix.toUtf8() + dataToSend;
        networkManager->sendData(finalData);

        statusBar()->showMessage("Data sent successfully: " + selectedOption, 3000);
        qDebug() << "Data sent successfully, total size:" << finalData.size() << "bytes";

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("Failed to send data: %1").arg(e.what()));
        qDebug() << "Error sending data:" << e.what();
    }
}

// void MainWindow::hSendData() {
//     // Check if connected
//     if (!networkManager->isConnected()) {
//         QMessageBox::warning(this, "Error", "You are not connected to a server or client.");
//         return;
//     }
//
//     // Create send options dialog
//     QDialog dialog(this);
//     dialog.setWindowTitle("Send Data");
//     dialog.setMinimumWidth(300);
//
//     QVBoxLayout* layout = new QVBoxLayout(&dialog);
//     QLabel* label = new QLabel("What do you want to do?", &dialog);
//     QComboBox* comboBox = new QComboBox(&dialog);
//     comboBox->addItem("Send voltage source to a specific node");
//     comboBox->addItem("Send the whole circuit file");
//     comboBox->addItem("Send a signal as an input to a circuit");
//     comboBox->addItem("Send component to circuit");
//
//     QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
//
//     layout->addWidget(label);
//     layout->addWidget(comboBox);
//     layout->addWidget(buttonBox);
//
//     connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
//     connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
//
//     if (dialog.exec() != QDialog::Accepted) {
//         return;
//     }
//
//     QString selectedOption = comboBox->currentText();
//     QByteArray dataToSend;
//
//     try {
//         if (selectedOption == "Send voltage source to a specific node") {
//             bool ok;
//             QString nodeName = QInputDialog::getText(this, "Node Name", "Enter node name:", QLineEdit::Normal, "", &ok);
//             if (!ok || nodeName.isEmpty()) return;
//
//             double voltage = QInputDialog::getDouble(this, "Voltage Value", "Enter voltage value:", 0, -1000, 1000, 2, &ok);
//             if (!ok) return;
//
//             QString message = QString("VOLTAGE_NODE %1 %2").arg(nodeName).arg(voltage);
//             dataToSend = message.toUtf8();
//         }
//         else if (selectedOption == "Send the whole circuit file") {
//             // Ensure project is saved first
//             saveProject();
//
//             QFile file("project.log.txt");
//             if (!file.open(QIODevice::ReadOnly)) {
//                 QMessageBox::warning(this, "Error", "Could not open circuit file for reading.");
//                 return;
//             }
//
//             dataToSend = "CIRCUIT:" + file.readAll();
//             file.close();
//         }
//         else if (selectedOption == "Send a signal as an input to a circuit") {
//             QString fileName = QFileDialog::getOpenFileName(this, "Select Signal File",
//                                                           QCoreApplication::applicationDirPath(),
//                                                           "Text Files (*.txt);;All Files (*)");
//             if (fileName.isEmpty()) return;
//
//             QFile file(fileName);
//             if (!file.open(QIODevice::ReadOnly)) {
//                 QMessageBox::warning(this, "Error", "Could not open signal file.");
//                 return;
//             }
//
//             dataToSend = "SIGNAL_INPUT:" + file.readAll();
//             file.close();
//         }
//         else if (selectedOption == "Send component to circuit") {
//             // Example implementation - you might want to expand this
//             QString componentData = "COMPONENT:Basic implementation - extend as needed";
//             dataToSend = componentData.toUtf8();
//         }
//
//         // Send the data
//         networkManager->sendData(dataToSend);
//         statusBar()->showMessage("Data sent successfully", 3000);
//
//     } catch (const std::exception& e) {
//         QMessageBox::warning(this, "Error", QString("Failed to send data: %1").arg(e.what()));
//     }
// }


// Implement the data received handler:
// Replace the onDataReceived method with this improved version:
void MainWindow::onDataReceived(const QByteArray& data, const QString& type) {
    qDebug() << "Data received, type:" << type << "size:" << data.size() << "bytes";

    QString message;
    QString filePath;

    try {
        if (type == "circuit") {
            // Ask user where to save the circuit file
            QString fileName = QFileDialog::getSaveFileName(
                this,
                "Save Received Circuit",
                QCoreApplication::applicationDirPath() + "/received_circuit.psp",
                "ParsaSpice Files (*.psp);;All Files (*)"
            );

            if (fileName.isEmpty()) {
                qDebug() << "User cancelled circuit file save";
                return;
            }

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                filePath = fileName;
                message = QString("Circuit file received and saved as: %1").arg(fileName);
            } else {
                message = "Failed to save received circuit file";
            }
        }
        else if (type == "signal") {
            // Ask user where to save the signal file
            QString fileName = QFileDialog::getSaveFileName(
                this,
                "Save Received Signal",
                QCoreApplication::applicationDirPath() + "/received_signal.txt",
                "Text Files (*.txt);;All Files (*)"
            );

            if (fileName.isEmpty()) {
                qDebug() << "User cancelled signal file save";
                return;
            }

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                filePath = fileName;
                message = QString("Signal file received and saved as: %1").arg(fileName);
            } else {
                message = "Failed to save received signal file";
            }
        }
        else if (type == "voltage") {
            QString voltageData = QString::fromUtf8(data);
            QStringList parts = voltageData.split(' ');
            if (parts.size() >= 2) {
                QString nodeName = parts[0];
                double voltage = parts[1].toDouble();
                message = QString("Voltage source received: %1V at node %2").arg(voltage).arg(nodeName);

                // You can add code here to automatically create the voltage source
                // circuit->addComponent("V", "Received_Vsource", nodeName, "GND", voltage, {}, {}, false);
            } else {
                message = "Invalid voltage data received: " + voltageData;
            }
        }
        else {
            message = "Unknown data type received: " + type;
            qDebug() << "Unknown data content:" << QString::fromUtf8(data);
        }

        // Show notification
        statusBar()->showMessage(message, 5000);

        // Show dialog only for important messages
        if (!filePath.isEmpty() || type == "voltage") {
            QMessageBox::information(this, "Data Received", message);
        }

        qDebug() << "Data processing completed:" << message;

    } catch (const std::exception& e) {
        QString errorMsg = QString("Error processing received data: %1").arg(e.what());
        statusBar()->showMessage(errorMsg, 5000);
        QMessageBox::warning(this, "Error", errorMsg);
        qDebug() << "Error in onDataReceived:" << e.what();
    }
}
// void MainWindow::onDataReceived(const QByteArray& data, const QString& type) {
//     QString message;
//
//     if (type == "circuit") {
//         // Save received circuit data
//         QFile file("received_circuit.log.txt");
//         if (file.open(QIODevice::WriteOnly)) {
//             file.write(data);
//             file.close();
//             message = "Circuit file received and saved";
//
//             // Optional: Automatically load the received circuit
//             // loadProject("received_circuit.log.txt");
//         } else {
//             message = "Failed to save received circuit file";
//         }
//     }
//     else if (type == "voltage") {
//         QString voltageData = QString::fromUtf8(data);
//         QStringList parts = voltageData.split(' ');
//         if (parts.size() >= 3 && parts[0] == "VOLTAGE_NODE") {
//             QString nodeName = parts[1];
//             double voltage = parts[2].toDouble();
//             message = QString("Voltage source received: %1V at node %2").arg(voltage).arg(nodeName);
//
//             // Here you would typically add the voltage source to your circuit
//             //circuit.addComponent(Component::Type::VOLTAGE_SOURCE, "name", "node1", "node2", "value", const std::vector<double>& numericParams, const std::vector<std::string>& stringParams, bool isSinusoidal) {
//  //addVoltageSource(nodeName, voltage);
//         }
//     }
//     else if (type == "signal") {
//         QFile file("received_signal.txt");
//         if (file.open(QIODevice::WriteOnly)) {
//             file.write(data);
//             file.close();
//             message = "Signal data received and saved";
//         } else {
//             message = "Failed to save received signal data";
//         }
//     }
//     else if (type == "component") {
//         message = "Component data received: " + QString::fromUtf8(data);
//         // Handle component data
//     }
//     else {
//         message = "Unknown data type received: " + QString::fromUtf8(data);
//     }
//
//     // Show notification
//     statusBar()->showMessage(message, 5000);
//     QMessageBox::information(this, "Data Received", message);
// }