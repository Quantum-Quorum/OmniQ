//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include "CoreInterface.h"
#include "widgets/CircuitView.h"
#include "widgets/QuantumStateViewer.h"
#include "widgets/QubitViewer.h"
#include "widgets/CircuitBuilder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newCircuit();
    void openCircuit();
    void saveCircuit();
    void saveCircuitAs();
    void stepForward();
    void stepBackward();
    void runCircuit();
    void pauseCircuit();
    void resetCircuit();
    void about();
    void updateStatus();
    void onGateAdded(const QString& gateType, int qubit, double parameter);

private:
    void setupUI();
    void setupMenus();
    void setupToolbars();
    void setupDockWidgets();
    void setupCircuitBuilder();
    void setupStatusBar();
    void createActions();
    void loadSettings();
    void saveSettings();

    // UI Components
    QSplitter *mainSplitter;
    CircuitView *circuitView;
    QuantumStateViewer *stateViewer;
    QubitViewer *qubitViewer;
    CircuitBuilder *circuitBuilder;
    
    // Dock Widgets
    QDockWidget *circuitDock;
    QDockWidget *stateDock;
    QDockWidget *qubitDock;
    QDockWidget *outputDock;
    
    // Controls
    QToolBar *debugToolBar;
    QPushButton *stepForwardBtn;
    QPushButton *stepBackwardBtn;
    QPushButton *runBtn;
    QPushButton *pauseBtn;
    QPushButton *resetBtn;
    QSpinBox *stepSpinBox;
    QComboBox *speedComboBox;
    QProgressBar *progressBar;
    
    // Core Interface
    CoreInterface *coreInterface;
    
    // Actions
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;
    QAction *stepForwardAction;
    QAction *stepBackwardAction;
    QAction *runAction;
    QAction *pauseAction;
    QAction *resetAction;
    QAction *aboutAction;
    
    // State
    bool isRunning;
    int currentStep;
    int totalSteps;
    QTimer *updateTimer;
};

#endif // MAINWINDOW_H
