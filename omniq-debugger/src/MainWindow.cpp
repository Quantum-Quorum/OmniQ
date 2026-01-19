//
// Created by Goutham Arcot on 17/07/25.
//

#include "MainWindow.h"
#include "widgets/ClickableLabel.h"
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QVector>

QTextEdit *MainWindow::s_outputConsole = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), mainSplitter(nullptr), circuitView(nullptr),
      stateViewer(nullptr), qubitViewer(nullptr), circuitDock(nullptr),
      stateDock(nullptr), qubitDock(nullptr), outputDock(nullptr),
      debugToolBar(nullptr), stepForwardBtn(nullptr), stepBackwardBtn(nullptr),
      runBtn(nullptr), pauseBtn(nullptr), resetBtn(nullptr),
      stepSpinBox(nullptr), speedComboBox(nullptr), progressBar(nullptr),
      newAction(nullptr), openAction(nullptr), saveAction(nullptr),
      saveAsAction(nullptr), exitAction(nullptr), stepForwardAction(nullptr),
      stepBackwardAction(nullptr), runAction(nullptr), pauseAction(nullptr),
      resetAction(nullptr), aboutAction(nullptr), isRunning(false),
      currentStep(0), totalSteps(0), updateTimer(nullptr) {
  // Initialize Core Interface
  coreInterface = new CoreInterface(this);
  // Handler will be installed after UI is ready in showEvent

  setupUI();
  createActions(); // Must create actions BEFORE using them in menus!
  setupMenus();
  setupToolbars();
  setupDockWidgets();
  setupCircuitBuilder();
  setupStatusBar();
  loadSettings();

  // Initialize Animation
  animationTimer_ = new QTimer(this);
  animationPhase_ = 0.0;
  isAnimating_ = false;
  connect(animationTimer_, &QTimer::timeout, this,
          &MainWindow::onUpdateAnimation);

  // Connect CoreInterface signals to UI
  connect(coreInterface, &CoreInterface::stateChanged, this,
          &MainWindow::updateStateDisplays);
  connect(coreInterface, &CoreInterface::circuitChanged, this,
          &MainWindow::syncCircuitWithBackend);
  connect(coreInterface, &CoreInterface::errorOccurred, this,
          [this](const QString &error) { logMessage(error, true); });
  setupLogging();

  qDebug() << "MainWindow initialized";
  logMessage("Quantum Debugger Initialized. Ready for simulation.");

  // Connect local widget signals to global slots
  connect(stateViewer, &QuantumStateViewer::animateToggled, this,
          &MainWindow::onAnimateToggled);

  updateTimer = new QTimer(this);
  connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
  updateTimer->start(100);

  setWindowTitle("OmniQ Quantum Circuit Debugger");
  resize(1200, 800);
}

MainWindow::~MainWindow() { saveSettings(); }

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  static bool handlerInstalled = false;
  if (!handlerInstalled) {
    qInstallMessageHandler(MainWindow::qtMessageHandler);
    handlerInstalled = true;
    qDebug() << "Global message handler installed after UI shown.";
  }
}

void MainWindow::setupUI() {
  // Use a central scroll area for the circuit view
  QScrollArea *centralScrollArea = new QScrollArea(this);
  centralScrollArea->setWidgetResizable(true);

  circuitView = new CircuitView(centralScrollArea);
  centralScrollArea->setWidget(circuitView);
  setCentralWidget(centralScrollArea);

  // Initialize other widgets but don't add to layout yet
  stateViewer = new QuantumStateViewer(this);
  qubitViewer = new QubitViewer(this);
  circuitBuilder = new CircuitBuilder(this);
  blochSphereWidget = new BlochSphereWidget(this);

  qDebug() << "MainWindow UI components initialized";
}

void MainWindow::setupMenus() {
  QMenu *fileMenu = menuBar()->addMenu("&File");
  if (newAction)
    fileMenu->addAction(newAction);
  if (openAction)
    fileMenu->addAction(openAction);
  if (saveAction)
    fileMenu->addAction(saveAction);
  if (saveAsAction)
    fileMenu->addAction(saveAsAction);
  fileMenu->addSeparator();
  if (exitAction)
    fileMenu->addAction(exitAction);

  QMenu *debugMenu = menuBar()->addMenu("&Debug");
  if (stepForwardAction)
    debugMenu->addAction(stepForwardAction);
  if (stepBackwardAction)
    debugMenu->addAction(stepBackwardAction);
  debugMenu->addSeparator();
  if (runAction)
    debugMenu->addAction(runAction);
  if (pauseAction)
    debugMenu->addAction(pauseAction);
  if (resetAction)
    debugMenu->addAction(resetAction);

  QMenu *helpMenu = menuBar()->addMenu("&Help");
  if (aboutAction)
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolbars() {
  debugToolBar = addToolBar("Debug");
  if (stepBackwardAction)
    debugToolBar->addAction(stepBackwardAction);
  if (stepForwardAction)
    debugToolBar->addAction(stepForwardAction);
  debugToolBar->addSeparator();
  if (runAction)
    debugToolBar->addAction(runAction);
  if (pauseAction)
    debugToolBar->addAction(pauseAction);
  if (resetAction)
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

void MainWindow::setupDockWidgets() {
  // Circuit builder dock
  circuitDock = new QDockWidget("Circuit Builder", this);
  QWidget *circuitHeader = new QWidget();
  QHBoxLayout *circuitHeaderLayout = new QHBoxLayout(circuitHeader);
  circuitHeaderLayout->setContentsMargins(5, 2, 5, 2);
  circuitHeaderLayout->addWidget(new QLabel("Circuit Builder"));
  circuitHeaderLayout->addStretch();
  circuitHeaderLayout->addWidget(createInfoIcon(
      "<b>Circuit Builder</b><br/>"
      "Constructs the quantum circuit structure. Gates dragged here "
      "instantiate <code>Gate</code> objects in the <code>Circuit</code> "
      "model. "
      "Execution triggers <code>CoreInterface::executeStep</code>."));
  circuitDock->setTitleBarWidget(circuitHeader);
  circuitDock->setWidget(circuitBuilder);
  addDockWidget(Qt::LeftDockWidgetArea, circuitDock);

  // Qubit viewer dock (grouped with Circuit Builder on the left)
  qubitDock = new QDockWidget("Qubit Details", this);
  QWidget *qubitHeader = new QWidget();
  QHBoxLayout *qubitHeaderLayout = new QHBoxLayout(qubitHeader);
  qubitHeaderLayout->setContentsMargins(5, 2, 5, 2);
  qubitHeaderLayout->addWidget(new QLabel("Qubit Details"));
  qubitHeaderLayout->addStretch();
  qubitHeaderLayout->addWidget(
      createInfoIcon("<b>Qubit Details</b><br/>"
                     "Live display of individual qubit statistics derived from "
                     "the <code>Statevector</code>. "
                     "Shows |0⟩/|1⟩ probabilities ($|\\alpha|^2, |\\beta|^2$) "
                     "and phase angles."));
  qubitDock->setTitleBarWidget(qubitHeader);
  qubitDock->setWidget(qubitViewer);
  addDockWidget(Qt::LeftDockWidgetArea, qubitDock);
  splitDockWidget(circuitDock, qubitDock, Qt::Vertical);

  // State Viewer dock
  stateDock = new QDockWidget("State Analyzer", this);
  stateDock->setWidget(stateViewer);
  addDockWidget(Qt::RightDockWidgetArea, stateDock);

  // Bloch Sphere dock (grouped with State Analyzer on the right)
  blochDock = new QDockWidget("3D Bloch Sphere", this);
  QWidget *blochHeader = new QWidget();
  QHBoxLayout *blochHeaderLayout = new QHBoxLayout(blochHeader);
  blochHeaderLayout->setContentsMargins(5, 2, 5, 2);
  blochHeaderLayout->addWidget(new QLabel("3D Bloch Sphere"));
  blochHeaderLayout->addStretch();
  blochHeaderLayout->addWidget(createInfoIcon(
      "<b>3D Bloch Sphere</b><br/>"
      "Geometric representation of a single qubit's pure state. "
      "Visualizes the superposition $\\psi = \\cos(\\theta/2)|0\\rangle + "
      "e^{i\\phi}\\sin(\\theta/2)|1\\rangle$ "
      "on the unit sphere."));
  blochDock->setTitleBarWidget(blochHeader);
  blochDock->setWidget(blochSphereWidget);
  addDockWidget(Qt::RightDockWidgetArea, blochDock);
  splitDockWidget(stateDock, blochDock, Qt::Vertical);

  // Output dock
  outputDock = new QDockWidget("Output", this);
  outputConsole_ = new QTextEdit(this);
  outputConsole_->setReadOnly(true);
  outputConsole_->setStyleSheet(
      "background-color: #1e1e1e; color: #d4d4d4; font-family: 'Consolas', "
      "'Monaco', monospace;");
  outputDock->setWidget(outputConsole_);
  s_outputConsole = outputConsole_;
  addDockWidget(Qt::BottomDockWidgetArea, outputDock);
}

void MainWindow::setupLogging() {
  connect(coreInterface, &CoreInterface::circuitChanged, this,
          [this]() { logMessage("Circuit updated."); });
  connect(coreInterface, &CoreInterface::stateChanged, this,
          [this]() { logMessage("Quantum state updated."); });
}

void MainWindow::logMessage(const QString &message, bool isError) {
  if (!outputConsole_)
    return;
  QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
  QString color = isError ? "#f48771" : "#75beff";
  outputConsole_->append(QString("<span style='color: #808080;'>[%1]</span> "
                                 "<span style='color: %2;'>%3</span>")
                             .arg(timestamp, color, message));
}

void MainWindow::qtMessageHandler(QtMsgType type,
                                  const QMessageLogContext &context,
                                  const QString &msg) {
  if (!s_outputConsole) {
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    return;
  }

  // Prevent re-entrancy if logMessage itself triggers a message
  static bool isLogging = false;
  if (isLogging) {
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    return;
  }
  isLogging = true;

  QString message = msg;

  QString color = "#d4d4d4";
  switch (type) {
  case QtDebugMsg:
    color = "#75beff";
    break;
  case QtInfoMsg:
    color = "#b5cea8";
    break;
  case QtWarningMsg:
    color = "#cca700";
    break;
  case QtCriticalMsg:
    color = "#f48771";
    break;
  case QtFatalMsg:
    color = "#f48771";
    break;
  }

  QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
  s_outputConsole->append(
      QString("<span style='color: #808080;'>[%1]</span> <span "
              "style='color: %2;'>%3</span>")
          .arg(timestamp, color, msg));
}

QLabel *MainWindow::createInfoIcon(const QString &tooltip) {
  ClickableLabel *infoIcon = new ClickableLabel("ⓘ", tooltip, this);
  return infoIcon;
}

void MainWindow::setupCircuitBuilder() {
  connect(circuitBuilder, &CircuitBuilder::gateAdded, this,
          &MainWindow::onGateAdded);
  connect(circuitBuilder, &CircuitBuilder::circuitChanged, this,
          [this]() { statusBar()->showMessage("Circuit updated"); });
}

void MainWindow::setupStatusBar() { statusBar()->showMessage("Ready"); }

void MainWindow::createActions() {
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
  connect(stepForwardAction, &QAction::triggered, this,
          &MainWindow::stepForward);

  stepBackwardAction = new QAction("Step Backward", this);
  stepBackwardAction->setShortcut(QKeySequence("Shift+F10"));
  connect(stepBackwardAction, &QAction::triggered, this,
          &MainWindow::stepBackward);

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

void MainWindow::newCircuit() {
  // TODO: have to implement new circuit creation
  statusBar()->showMessage("New circuit created");
}

void MainWindow::openCircuit() {
  QString fileName =
      QFileDialog::getOpenFileName(this, "Open Quantum Circuit", "",
                                   "Circuit Files (*.json);;All Files (*)");
  if (!fileName.isEmpty()) {
    loadCircuit(fileName);
  }
}

void MainWindow::loadCircuit(const QString &fileName) {
  if (coreInterface->loadCircuit(fileName)) {
    coreInterface->executeFull(); // Run all gates to get the final state
    syncCircuitWithBackend();

    // Apply recommended view mode if present
    QString viewMode = coreInterface->getRecommendedViewMode();
    if (!viewMode.isEmpty() && stateViewer) {
      stateViewer->setViewMode(viewMode);
    }

    statusBar()->showMessage("Loaded circuit: " + fileName);
    qDebug() << "Circuit loaded and executed. State vector size:"
             << coreInterface->getStateVectorComplex().size();
  }
}

void MainWindow::updateStateDisplays() {
  if (!coreInterface || !stateViewer || !qubitViewer || !blochSphereWidget)
    return;

  // Get current state vector
  rawStateVector_ = coreInterface->getStateVectorComplex();
  if (rawStateVector_.isEmpty())
    return;

  // Update State Viewer (Static by default now)
  stateViewer->updateStateVector(rawStateVector_);

  // Update Density Matrix if applicable
  stateViewer->updateDensityMatrix(coreInterface->getDensityMatrixComplex());

  // Update Qubit Viewer
  QVector<double> qstate = coreInterface->getQubitState(0);
  if (!qstate.isEmpty()) {
    qubitViewer->updateQubitInfo(0, qstate[0], qstate[1], qstate[2], qstate[4],
                                 qstate[6], qstate[7]);
  }

  // Update Bloch Sphere
  std::complex<double> alpha(qstate[2], qstate[3]);
  std::complex<double> beta(qstate[4], qstate[5]);
  blochSphereWidget->setQuantumState(alpha, beta);
}

void MainWindow::onAnimateToggled(bool checked) {
  isAnimating_ = checked;
  if (isAnimating_) {
    animationTimer_->start(50);
  } else {
    animationTimer_->stop();
    updateStateDisplays(); // Reset to static state
  }
}

void MainWindow::onUpdateAnimation() {
  animationPhase_ += 0.1;

  if (rawStateVector_.isEmpty())
    return;

  // Apply animation effect
  QVector<std::complex<double>> animatedState = rawStateVector_;
  for (int i = 0; i < animatedState.size(); ++i) {
    double phase = std::arg(animatedState[i]) + 0.3 * std::sin(animationPhase_);
    double mag = std::abs(animatedState[i]);
    animatedState[i] =
        std::complex<double>(mag * std::cos(phase), mag * std::sin(phase));
  }

  // Only animate the visualizations, while keeping numeric data/circuit stable
  // This addresses user's preference for animation to be in the "visuls"
  // sections

  // 1. Update Bloch Sphere (animate its state)
  // For a single qubit, we derive the phase shift from the animated vector
  if (rawStateVector_.size() >= 2) {
    std::complex<double> a = animatedState[0];
    std::complex<double> b = animatedState[1];
    blochSphereWidget->setQuantumState(a, b);
  }

  // 2. Optionally animate the 3D bars/graph if they are visible
  // We can pass the animated state to stateViewer but tell it not to update
  // text
  stateViewer->updateStateVector(animatedState);
}

void MainWindow::syncCircuitWithBackend() {
  if (!coreInterface || !circuitView)
    return;

  // Update visual circuit based on backend
  circuitView->clear();
  int numQubits = coreInterface->getStateVectorComplex().isEmpty()
                      ? 2
                      : static_cast<int>(std::log2(
                            coreInterface->getStateVectorComplex().size()));
  circuitView->setNumQubits(numQubits);

  // Refresh the display
  updateStateDisplays();
}

void MainWindow::saveCircuit() {
  // TODO: have to implement circuit saving
  statusBar()->showMessage("Circuit saved");
}

void MainWindow::saveCircuitAs() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "Save Quantum Circuit", "",
      "Circuit Files (*.qasm *.json);;All Files (*)");
  if (!fileName.isEmpty()) {
    // TODO: have to implement circuit saving
    statusBar()->showMessage("Circuit saved as: " + fileName);
  }
}

void MainWindow::stepForward() {
  if (currentStep < totalSteps) {
    currentStep++;
    if (stepSpinBox) {
      stepSpinBox->setValue(currentStep);
    }
    // TODO: have to update circuit state
    statusBar()->showMessage(
        QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
  }
}

void MainWindow::stepBackward() {
  if (currentStep > 0) {
    currentStep--;
    if (stepSpinBox) {
      stepSpinBox->setValue(currentStep);
    }
    // TODO: have to update circuit state
    statusBar()->showMessage(
        QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
  }
}

void MainWindow::runCircuit() {
  isRunning = true;
  if (runAction)
    runAction->setEnabled(false);
  if (pauseAction)
    pauseAction->setEnabled(true);
  statusBar()->showMessage("Running circuit...");
  // TODO: have to implement circuit execution
}

void MainWindow::pauseCircuit() {
  isRunning = false;
  if (runAction)
    runAction->setEnabled(true);
  if (pauseAction)
    pauseAction->setEnabled(false);
  statusBar()->showMessage("Circuit paused");
}

void MainWindow::resetCircuit() {
  currentStep = 0;
  if (stepSpinBox)
    stepSpinBox->setValue(0);
  isRunning = false;
  if (runAction)
    runAction->setEnabled(true);
  if (pauseAction)
    pauseAction->setEnabled(false);
  statusBar()->showMessage("Circuit reset");
  // TODO: have to reset circuit state
}

void MainWindow::onGateAdded(const QString &gateType, int qubit,
                             double parameter) {
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

    statusBar()->showMessage(
        QString("Added %1 gate to qubit %2").arg(gateType).arg(qubit));
  }
}

void MainWindow::about() {
  QMessageBox::about(this, "About OmniQ Debugger",
                     "OmniQ Quantum Circuit Debugger\n\n"
                     "Version 0.1.0\n"
                     "A graphical debugger for quantum circuits\n\n"
                     "© 2024 Quantum-Quorum");
}

void MainWindow::updateStatus() {
  if (totalSteps > 0 && progressBar) {
    int progress = (currentStep * 100) / totalSteps;
    progressBar->setValue(progress);
  }
}

void MainWindow::loadSettings() {
  QSettings settings;
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::saveSettings() {
  QSettings settings;
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
}
