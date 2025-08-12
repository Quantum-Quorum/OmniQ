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
    , mainSplitter(nullptr)
    , circuitView(nullptr)
    , stateViewer(nullptr)
    , qubitViewer(nullptr)
    , circuitDock(nullptr)
    , stateDock(nullptr)
    , qubitDock(nullptr)
    , outputDock(nullptr)
    , debugToolBar(nullptr)
    , stepForwardBtn(nullptr)
    , stepBackwardBtn(nullptr)
    , runBtn(nullptr)
    , pauseBtn(nullptr)
    , resetBtn(nullptr)
    , stepSpinBox(nullptr)
    , speedComboBox(nullptr)
    , progressBar(nullptr)
    , coreInterface(new CoreInterface(this))
    , newAction(nullptr)
    , openAction(nullptr)
    , saveAction(nullptr)
    , saveAsAction(nullptr)
    , exitAction(nullptr)
    , stepForwardAction(nullptr)
    , stepBackwardAction(nullptr)
    , runAction(nullptr)
    , pauseAction(nullptr)
    , resetAction(nullptr)
    , aboutAction(nullptr)
    , isRunning(false)
    , currentStep(0)
    , totalSteps(0)
    , updateTimer(nullptr)
{
    setupUI();
    setupMenus();
    setupToolbars();
    setupDockWidgets();
    setupCircuitBuilder();
    setupStatusBar();
    createActions();
    loadSettings();
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    updateTimer->start(100);
    
    setWindowTitle("OmniQ Quantum Circuit Debugger");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);
    
    circuitView = new CircuitView(this);
    stateViewer = new QuantumStateViewer(this);
    qubitViewer = new QubitViewer(this);
    circuitBuilder = new CircuitBuilder(this);
    
    mainSplitter->addWidget(circuitBuilder);
    mainSplitter->addWidget(circuitView);
    mainSplitter->addWidget(stateViewer);
    mainSplitter->setStretchFactor(0, 2);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setStretchFactor(2, 1);
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    QMenu *debugMenu = menuBar()->addMenu("&Debug");
    debugMenu->addAction(stepForwardAction);
    debugMenu->addAction(stepBackwardAction);
    debugMenu->addSeparator();
    debugMenu->addAction(runAction);
    debugMenu->addAction(pauseAction);
    debugMenu->addAction(resetAction);
    
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolbars()
{
    debugToolBar = addToolBar("Debug");
    debugToolBar->addAction(stepBackwardAction);
    debugToolBar->addAction(stepForwardAction);
    debugToolBar->addSeparator();
    debugToolBar->addAction(runAction);
    debugToolBar->addAction(pauseAction);
    debugToolBar->addAction(resetAction);
    debugToolBar->addSeparator();
    
    debugToolBar->addWidget(new QLabel("Step:"));
    stepSpinBox = new QSpinBox(this);
    stepSpinBox->setRange(0, 1000);
    stepSpinBox->setValue(0);
    debugToolBar->addWidget(stepSpinBox);
    
    debugToolBar->addWidget(new QLabel("Speed:"));
    speedComboBox = new QComboBox(this);
    speedComboBox->addItems({"Slow", "Normal", "Fast"});
    speedComboBox->setCurrentText("Normal");
    debugToolBar->addWidget(speedComboBox);
    
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

void MainWindow::setupCircuitBuilder()
{
    connect(circuitBuilder, &CircuitBuilder::gateAdded, this, &MainWindow::onGateAdded);
    connect(circuitBuilder, &CircuitBuilder::circuitChanged, this, [this]() {
        statusBar()->showMessage("Circuit updated");
    });
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
    // TODO: have to implement new circuit creation
    statusBar()->showMessage("New circuit created");
}

void MainWindow::openCircuit()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Open Quantum Circuit", "", "Circuit Files (*.qasm *.json);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: have to implement circuit loading
        statusBar()->showMessage("Opened circuit: " + fileName);
    }
}

void MainWindow::saveCircuit()
{
    // TODO: have to implement circuit saving
    statusBar()->showMessage("Circuit saved");
}

void MainWindow::saveCircuitAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Quantum Circuit", "", "Circuit Files (*.qasm *.json);;All Files (*)");
    if (!fileName.isEmpty()) {
        // TODO: have to implement circuit saving
        statusBar()->showMessage("Circuit saved as: " + fileName);
    }
}

void MainWindow::stepForward()
{
    if (currentStep < totalSteps) {
        currentStep++;
        if (stepSpinBox) {
            stepSpinBox->setValue(currentStep);
        }
        // TODO: have to update circuit state
        statusBar()->showMessage(QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
    }
}

void MainWindow::stepBackward()
{
    if (currentStep > 0) {
        currentStep--;
        if (stepSpinBox) {
            stepSpinBox->setValue(currentStep);
        }
        // TODO: have to update circuit state
        statusBar()->showMessage(QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
    }
}

void MainWindow::runCircuit()
{
    isRunning = true;
    if (runAction) runAction->setEnabled(false);
    if (pauseAction) pauseAction->setEnabled(true);
    statusBar()->showMessage("Running circuit...");
    // TODO: have to implement circuit execution    
}

void MainWindow::pauseCircuit()
{
    isRunning = false;
    if (runAction) runAction->setEnabled(true);
    if (pauseAction) pauseAction->setEnabled(false);
    statusBar()->showMessage("Circuit paused");
}

void MainWindow::resetCircuit()
{
    currentStep = 0;
    if (stepSpinBox) stepSpinBox->setValue(0);
    isRunning = false;
    if (runAction) runAction->setEnabled(true);
    if (pauseAction) pauseAction->setEnabled(false);
    statusBar()->showMessage("Circuit reset");
    // TODO: have to reset circuit state
}

void MainWindow::onGateAdded(const QString& gateType, int qubit, double parameter)
{
    if (coreInterface) {
        QVector<int> qubits;
        QVector<double> parameters;
        // Convert gate type to backend format
        if (gateType == "H") {
            qubits = {qubit};
            coreInterface->addGate("H", qubits);
        } else if (gateType == "X") {
            qubits = {qubit};
            coreInterface->addGate("X", qubits);
        } else if (gateType == "Y") {
            qubits = {qubit};
            coreInterface->addGate("Y", qubits);
        } else if (gateType == "Z") {
            qubits = {qubit};
            coreInterface->addGate("Z", qubits);
        } else if (gateType == "CNOT") {
            qubits = {qubit, qubit + 1};
            coreInterface->addGate("CNOT", qubits);
        } else if (gateType == "SWAP") {
            qubits = {qubit, qubit + 1};
            coreInterface->addGate("SWAP", qubits);
        } else if (gateType == "RX") {
            qubits = {qubit};
            parameters = {parameter};
            coreInterface->addGate("RX", qubits, parameters);
        } else if (gateType == "RY") {
            qubits = {qubit};
            parameters = {parameter};
            coreInterface->addGate("RY", qubits, parameters);
        } else if (gateType == "RZ") {
            qubits = {qubit};
            parameters = {parameter};
            coreInterface->addGate("RZ", qubits, parameters);
        } else if (gateType == "PHASE") {
            qubits = {qubit};
            parameters = {parameter};
            coreInterface->addGate("PHASE", qubits, parameters);
        }
        
        statusBar()->showMessage(QString("Added %1 gate to qubit %2").arg(gateType).arg(qubit));
    }
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
    if (totalSteps > 0 && progressBar) {
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
