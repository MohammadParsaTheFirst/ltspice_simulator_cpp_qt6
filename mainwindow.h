#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QApplication> // for quiting the program
#include <QPixmap>
#include <QToolBar>
#include <QIcon>
#include <QFileDialog>
#include <QInputDialog>

#include "ui_mainwindow.h"
#include "SchematicWidget.h"
#include "Circuit.h"
#include "NetworkManager.h"  // Add this include
#include "NetworkDialog.h"   // Add this include

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Ui::MainWindow* ui;
    SchematicWidget* schematic;
    Circuit circuit;
    QString currentProjectPath;
    QString schematicsPath;
    QString currentProjectName;
    NetworkManager* networkManager;  // Add this member

    void setupWelcomeState();
    void setupSchematicState(const QString& projectName = "Draft.asc");

    // Some items in menu bar to disable and enabling them
    QAction* sendAction; // Added
    QAction* settingsAction;
    QAction* newSchematicAction;
    QAction* saveAction;
    QAction* openAction;
    QAction* configureAnalysisAction;
    QAction* runAction;
    QAction* wireAction;
    QAction* groundAction;
    QAction* voltageSourceAction;
    QAction* resistorAction;
    QAction* capacitorAction;
    QAction* inductorAction;
    QAction* diodeAction;
    QAction* nodeLibraryAction;
    QAction* labelAction;
    QAction* deleteModeAction;
    QAction* createSubcircuitAction;
    QAction* subcircuitLibraryAction;
    QAction* quitAction;
    QAction* networkAction;  // Add this member
    QAction* sendFileAction;

    private slots:
        void hNewSchematic();
    void hSendFile();
    //void hSendData(); // Added
    void hShowSettings();
    void hSaveProject();
    void hOpenProject();
    void hNetworkConnection();
    void onNetworkStatusChanged(bool connected, const QString& message);                                                // Add this slot
    void onVoltageSourceReceived(const QString& name, const QString& node1, const QString& node2,                       // Add this slot
                               double value, bool isSinusoidal, double offset, double amplitude, double frequency);     // Add this slot
    void onCircuitFileReceived();                                                                                       // Add this slot
    void onSignalDataReceived(const std::map<double, double>& data, const QString& signalName);                         // Add this slot
    //void onDataReceived(const QByteArray& data, const QString& type); // Add this line
    void onFileReceived(const QString& fileName, const QByteArray& fileData);

public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow();

    void starterWindow();
    void initializeActions();
    void implementMenuBar();
    void implementToolBar();
    void shortcutRunner();
    void saveProject();

    void loadSubcircuitsFromLibrary();
};


#endif //MAINWINDOW_H