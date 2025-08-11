//
// Created by Goutham Arcot on 17/07/25.
//

#include "MainWindow.h"
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QTimer>
#include <QIcon>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isRunning(false)
    , currentStep(0)
    , totalSteps(0)
    , coreInterface(new CoreInterface(this))
{
    setupUI();
    setupMenus();
    setupToolbars();
    setupDockWidgets();
    setupStatusBar();
    createActions();
    loadSettings();
    
    // Setup update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    updateTimer->start(100); // Update every 100ms
    
    setWindowTitle("OmniQ Quantum Circuit Debugger");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    // Create main splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);
    
    // Create widgets
    circuitView = new CircuitView(this);
    stateViewer = new QuantumStateViewer(this);
    qubitViewer = new QubitViewer(this);
    
    // Add widgets to splitter
    mainSplitter->addWidget(circuitView);
    mainSplitter->addWidget(stateViewer);
    mainSplitter->setStretchFactor(0, 2);
    mainSplitter->setStretchFactor(1, 1);
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // Debug menu
    QMenu *debugMenu = menuBar()->addMenu("&Debug");
    debugMenu->addAction(stepForwardAction);
    debugMenu->addAction(stepBackwardAction);
    debugMenu->addSeparator();
    debugMenu->addAction(runAction);
    debugMenu->addAction(pauseAction);
    debugMenu->addAction(resetAction);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolbars()
{
    // Debug toolbar
    debugToolBar = addToolBar("Debug");
    debugToolBar->addAction(stepBackwardAction);
    debugToolBar->addAction(stepForwardAction);
    debugToolBar->addSeparator();
    debugToolBar->addAction(runAction);
    debugToolBar->addAction(pauseAction);
    debugToolBar->addAction(resetAction);
    debugToolBar->addSeparator();
    
    // Step control
    debugToolBar->addWidget(new QLabel("Step:"));
    stepSpinBox = new QSpinBox(this);
    stepSpinBox->setRange(0, 1000);
    stepSpinBox->setValue(0);
    debugToolBar->addWidget(stepSpinBox);
    
    // Speed control
    debugToolBar->addWidget(new QLabel("Speed:"));
    speedComboBox = new QComboBox(this);
    speedComboBox->addItems({"Slow", "Normal", "Fast"});
    speedComboBox->setCurrentText("Normal");
    debugToolBar->addWidget(speedComboBox);
    
    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    debugToolBar->addWidget(progressBar);
}

void MainWindow::setupDockWidgets()
{
    // Circuit dock
    circuitDock = new QDockWidget("Circuit", this);
    circuitDock->setWidget(circuitView);
    addDockWidget(Qt::LeftDockWidgetArea, circuitDock);
    
    // State viewer dock
    stateDock = new QDockWidget("Quantum State", this);
    stateDock->setWidget(stateViewer);
    addDockWidget(Qt::RightDockWidgetArea, stateDock);
    
    // Qubit viewer dock
    qubitDock = new QDockWidget("Qubit Details", this);
    qubitDock->setWidget(qubitViewer);
    addDockWidget(Qt::BottomDockWidgetArea, qubitDock);
    
    // Output dock
    outputDock = new QDockWidget("Output", this);
    QTextEdit *outputText = new QTextEdit(this);
    outputText->setReadOnly(true);
    outputDock->setWidget(outputText);
    addDockWidget(Qt::BottomDockWidgetArea, outputDock);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::createActions()
{
    // File actions
    newAction = new QAction("&New", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newCircuit);
    
    openAction = new QAction("&Open", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openCircuit);
    
    saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveCircuit);
    
    saveAsAction = new QAction("Save &As", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveCircuitAs);
    
    exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Debug actions
    stepForwardAction = new QAction("Step Forward", this);
    stepForwardAction->setShortcut(QKeySequence("F10"));
    connect(stepForwardAction, &QAction::triggered, this, &MainWindow::stepForward);
    
    stepBackwardAction = new QAction("Step Backward", this);
    stepBackwardAction->setShortcut(QKeySequence("Shift+F10"));
    connect(stepBackwardAction, &QAction::triggered, this, &MainWindow::stepBackward);
    
    runAction = new QAction("Run", this);
    runAction->setShortcut(QKeySequence("F5"));
    connect(runAction, &QAction::triggered, this, &MainWindow::runCircuit);
    
    pauseAction = new QAction("Pause", this);
    pauseAction->setShortcut(QKeySequence("F6"));
    connect(pauseAction, &QAction::triggered, this, &MainWindow::pauseCircuit);
    
    resetAction = new QAction("Reset", this);
    resetAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetCircuit);
    
    // Help actions
    aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::newCircuit()
{
    // TODO: Implement new circuit creation
    statusBar()->showMessage("New circuit created");
}

void MainWindow::openCircuit()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Quantum Circuit", "", "Circuit Files (*.qasm *.json);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: Implement circuit loading
        statusBar()->showMessage("Opened circuit: " + fileName);
    }
}

void MainWindow::saveCircuit()
{
    // TODO: Implement circuit saving
    statusBar()->showMessage("Circuit saved");
}

void MainWindow::saveCircuitAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Quantum Circuit", "", "Circuit Files (*.qasm *.json);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: Implement circuit saving
        statusBar()->showMessage("Circuit saved as: " + fileName);
    }
}

void MainWindow::stepForward()
{
    if (currentStep < totalSteps) {
        currentStep++;
        stepSpinBox->setValue(currentStep);
        // TODO: Update circuit state
        statusBar()->showMessage(QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
    }
}

void MainWindow::stepBackward()
{
    if (currentStep > 0) {
        currentStep--;
        stepSpinBox->setValue(currentStep);
        // TODO: Update circuit state
        statusBar()->showMessage(QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
    }
}

void MainWindow::runCircuit()
{
    isRunning = true;
    runAction->setEnabled(false);
    pauseAction->setEnabled(true);
    statusBar()->showMessage("Running circuit...");
    // TODO: Implement circuit execution
}

void MainWindow::pauseCircuit()
{
    isRunning = false;
    runAction->setEnabled(true);
    pauseAction->setEnabled(false);
    statusBar()->showMessage("Circuit paused");
}

void MainWindow::resetCircuit()
{
    currentStep = 0;
    stepSpinBox->setValue(0);
    isRunning = false;
    runAction->setEnabled(true);
    pauseAction->setEnabled(false);
    statusBar()->showMessage("Circuit reset");
    // TODO: Reset circuit state
}

void MainWindow::about()
{
    QMessageBox::about(this, "About OmniQ Debugger",
        "OmniQ Quantum Circuit Debugger\n\n"
        "Version 0.1.0\n"
        "A graphical debugger for quantum circuits\n\n"
        "© 2024 Quantum-Quorum");
}

void MainWindow::updateStatus()
{
    if (totalSteps > 0) {
        int progress = (currentStep * 100) / totalSteps;
        progressBar->setValue(progress);
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}
