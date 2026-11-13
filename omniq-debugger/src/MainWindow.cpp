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
      blochDock(nullptr), entanglementDock(nullptr), blochQubitCombo_(nullptr), blochRunBtn_(nullptr),
      activeBlochQubit_(0), debugToolBar(nullptr), stepForwardBtn(nullptr), stepBackwardBtn(nullptr),
      runBtn(nullptr), pauseBtn(nullptr), resetBtn(nullptr),
      stepSpinBox(nullptr), speedComboBox(nullptr), progressBar(nullptr),
      newAction(nullptr), openAction(nullptr), saveAction(nullptr),
      saveAsAction(nullptr), exitAction(nullptr), stepForwardAction(nullptr),
      stepBackwardAction(nullptr), runAction(nullptr), pauseAction(nullptr),
      resetAction(nullptr), resetLayoutAction(nullptr), aboutAction(nullptr),
      isRunning(false), currentStep(0), totalSteps(0), updateTimer(nullptr),
      runTimer_(nullptr) {
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

  // Initialize Circuit Runner Timer
  runTimer_ = new QTimer(this);
  connect(runTimer_, &QTimer::timeout, this, &MainWindow::onRunTimerTick);

  // Connect CoreInterface signals to UI
  connect(coreInterface, &CoreInterface::stateChanged, this,
          &MainWindow::updateStateDisplays);
  connect(coreInterface, &CoreInterface::circuitChanged, this,
          &MainWindow::syncCircuitWithBackend);
  connect(coreInterface, &CoreInterface::errorOccurred, this,
          [this](const QString &error) { logMessage(error, true); });
  setupLogging();

  // Create default circuit and sync
  coreInterface->createCircuit(2);
  syncCircuitWithBackend();

  qDebug() << "MainWindow initialized";
  logMessage("Quantum Debugger Initialized. Ready for simulation.");

  // Connect local widget signals to global slots
  connect(stateViewer, &QuantumStateViewer::animateToggled, this,
          &MainWindow::onAnimateToggled);

  updateTimer = new QTimer(this);
  connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
  updateTimer->start(100);

  setWindowTitle("OmniQ Quantum Circuit Debugger");
  resize(1380, 850);
}

MainWindow::~MainWindow() { saveSettings(); }

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  static bool firstShow = true;
  if (firstShow) {
    firstShow = false;
    qInstallMessageHandler(MainWindow::qtMessageHandler);
    qDebug() << "Global message handler installed after UI shown.";
    resizeDocks({circuitDock, qubitDock}, {340, 340}, Qt::Horizontal);
    resizeDocks({stateDock, blochDock}, {440, 440}, Qt::Horizontal);
  }
}

void MainWindow::setupUI() {
  // Use a central scroll area for the circuit view
  QScrollArea *centralScrollArea = new QScrollArea(this);
  centralScrollArea->setWidgetResizable(true);

  circuitView = new CircuitView(centralScrollArea);
  centralScrollArea->setWidget(circuitView);
  setCentralWidget(centralScrollArea);

  connect(circuitView, &CircuitView::stepSelected, this, [this](int step) {
    if (step >= 0 && step <= totalSteps) {
      currentStep = step;
      if (stepSpinBox) {
        stepSpinBox->blockSignals(true);
        stepSpinBox->setValue(currentStep);
        stepSpinBox->blockSignals(false);
      }
      coreInterface->executeStep(currentStep);
      circuitView->setCurrentStep(currentStep);
      updateStateDisplays();
      updateStatus();
      statusBar()->showMessage(QString("Jumped to step %1").arg(currentStep));
    }
  });

  // Initialize other widgets but don't add to layout yet
  stateViewer = new QuantumStateViewer(this);
  qubitViewer = new QubitViewer(this);
  circuitBuilder = new CircuitBuilder(this);
  blochSphereWidget = new BlochSphereWidget(this);
  entanglementViewer = new EntanglementViewer(coreInterface, this);

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

  QMenu *viewMenu = menuBar()->addMenu("&View");
  if (circuitDock)
    viewMenu->addAction(circuitDock->toggleViewAction());
  if (qubitDock)
    viewMenu->addAction(qubitDock->toggleViewAction());
  if (stateDock)
    viewMenu->addAction(stateDock->toggleViewAction());
  if (blochDock)
    viewMenu->addAction(blochDock->toggleViewAction());
  if (entanglementDock)
    viewMenu->addAction(entanglementDock->toggleViewAction());
  if (outputDock)
    viewMenu->addAction(outputDock->toggleViewAction());
  viewMenu->addSeparator();
  if (resetLayoutAction)
    viewMenu->addAction(resetLayoutAction);

  QMenu *helpMenu = menuBar()->addMenu("&Help");
  if (aboutAction)
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupToolbars() {
  debugToolBar = addToolBar("Debug");
  debugToolBar->setObjectName("debugToolBar");
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
  connect(stepSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
          [this](int val) {
            if (val != currentStep && val >= 0 && val <= totalSteps) {
              currentStep = val;
              coreInterface->executeStep(currentStep);
              circuitView->setCurrentStep(currentStep);
              updateStateDisplays();
              updateStatus();
              statusBar()->showMessage(
                  QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
            }
          });
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

static QScrollArea *makeScrollableDock(QWidget *content, QWidget *parent) {
  QScrollArea *scroll = new QScrollArea(parent);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scroll->setStyleSheet(
      "QScrollArea { background-color: #12151c; border: none; } "
      "QScrollBar:vertical { background: #181b24; width: 10px; margin: 0; } "
      "QScrollBar::handle:vertical { background: #3b4252; min-height: 20px; border-radius: 4px; } "
      "QScrollBar::handle:vertical:hover { background: #4c566a; } "
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; } "
      "QScrollBar:horizontal { background: #181b24; height: 10px; margin: 0; } "
      "QScrollBar::handle:horizontal { background: #3b4252; min-width: 20px; border-radius: 4px; } "
      "QScrollBar::handle:horizontal:hover { background: #4c566a; } "
      "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }");
  scroll->setWidget(content);
  return scroll;
}

void MainWindow::setupDockWidgets() {
  setDockNestingEnabled(true);
  setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowTabbedDocks |
                 QMainWindow::AllowNestedDocks);
  setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

  // 1. Circuit builder dock
  circuitDock = new QDockWidget("Circuit Builder", this);
  circuitDock->setObjectName("circuitDock");
  QWidget *circuitHeader = new QWidget();
  QHBoxLayout *circuitHeaderLayout = new QHBoxLayout(circuitHeader);
  circuitHeaderLayout->setContentsMargins(6, 4, 6, 4);
  circuitHeaderLayout->addWidget(new QLabel("Circuit Builder"));
  circuitHeaderLayout->addStretch();
  circuitHeaderLayout->addWidget(createInfoIcon(
      "<b>Circuit Builder</b><br/>"
      "Constructs the quantum circuit structure. Gates dragged here "
      "instantiate <code>Gate</code> objects in the <code>Circuit</code> "
      "model. Execution triggers <code>CoreInterface::executeStep</code>."));
  circuitDock->setTitleBarWidget(circuitHeader);
  circuitDock->setWidget(makeScrollableDock(circuitBuilder, this));

  // 2. Qubit viewer dock
  qubitDock = new QDockWidget("Qubit Details", this);
  qubitDock->setObjectName("qubitDock");
  QWidget *qubitHeader = new QWidget();
  QHBoxLayout *qubitHeaderLayout = new QHBoxLayout(qubitHeader);
  qubitHeaderLayout->setContentsMargins(6, 4, 6, 4);
  qubitHeaderLayout->addWidget(new QLabel("Qubit Details"));
  qubitHeaderLayout->addStretch();
  qubitHeaderLayout->addWidget(
      createInfoIcon("<b>Qubit Details</b><br/>"
                     "Live display of individual qubit statistics derived from "
                     "the <code>Statevector</code>. Shows |0⟩/|1⟩ probabilities "
                     "and phase angles."));
  qubitDock->setTitleBarWidget(qubitHeader);
  qubitDock->setWidget(makeScrollableDock(qubitViewer, this));

  // 3. State Viewer dock
  stateDock = new QDockWidget("State Analyzer", this);
  stateDock->setObjectName("stateDock");
  QWidget *stateHeader = new QWidget();
  QHBoxLayout *stateHeaderLayout = new QHBoxLayout(stateHeader);
  stateHeaderLayout->setContentsMargins(6, 4, 6, 4);
  stateHeaderLayout->addWidget(new QLabel("State Analyzer"));
  stateHeaderLayout->addStretch();
  stateHeaderLayout->addWidget(createInfoIcon(
      "<b>Quantum State Analyzer</b><br/>"
      "Visualizes the multi-qubit state vector, density matrix, purity, fidelity, "
      "Von Neumann entropy, and bipartite entanglement measures."));
  stateDock->setTitleBarWidget(stateHeader);
  stateDock->setWidget(makeScrollableDock(stateViewer, this));

  // 4. Bloch Sphere dock
  blochDock = new QDockWidget("3D Bloch Sphere", this);
  blochDock->setObjectName("blochDock");
  QWidget *blochHeader = new QWidget();
  QHBoxLayout *blochHeaderLayout = new QHBoxLayout(blochHeader);
  blochHeaderLayout->setContentsMargins(6, 4, 6, 4);
  blochHeaderLayout->setSpacing(6);

  QLabel *blochTitle = new QLabel("3D Bloch Sphere", blochHeader);
  blochTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #e2e8f0;");
  blochHeaderLayout->addWidget(blochTitle);

  blochHeaderLayout->addStretch();

  // Playback controls in dock header
  blochRunBtn_ = new QPushButton("▶ Run", blochHeader);
  blochRunBtn_->setToolTip("Run/Pause simulation and animate 3D Bloch trajectory");
  blochRunBtn_->setStyleSheet(
      "QPushButton { background-color: #059669; color: #ffffff; border: 1px solid #10b981; "
      "border-radius: 4px; padding: 2px 9px; font-weight: bold; font-size: 11px; } "
      "QPushButton:hover { background-color: #10b981; }");
  blochHeaderLayout->addWidget(blochRunBtn_);
  connect(blochRunBtn_, &QPushButton::clicked, this, [this]() {
    if (isRunning) pauseCircuit(); else runCircuit();
  });

  QPushButton *blochStepBtn = new QPushButton("Step", blochHeader);
  blochStepBtn->setToolTip("Step forward to next quantum gate");
  blochStepBtn->setStyleSheet(
      "QPushButton { background-color: #334155; color: #e2e8f0; border: 1px solid #475569; "
      "border-radius: 4px; padding: 2px 7px; font-size: 11px; } "
      "QPushButton:hover { background-color: #475569; }");
  blochHeaderLayout->addWidget(blochStepBtn);
  connect(blochStepBtn, &QPushButton::clicked, this, &MainWindow::stepForward);

  QPushButton *blochResetBtn = new QPushButton("Reset", blochHeader);
  blochResetBtn->setToolTip("Reset simulation to step 0");
  blochResetBtn->setStyleSheet(
      "QPushButton { background-color: #334155; color: #e2e8f0; border: 1px solid #475569; "
      "border-radius: 4px; padding: 2px 7px; font-size: 11px; } "
      "QPushButton:hover { background-color: #475569; }");
  blochHeaderLayout->addWidget(blochResetBtn);
  connect(blochResetBtn, &QPushButton::clicked, this, &MainWindow::resetCircuit);

  QLabel *qubitLbl = new QLabel("Qubit:", blochHeader);
  qubitLbl->setStyleSheet("color: #94a3b8; font-size: 11px;");
  blochHeaderLayout->addWidget(qubitLbl);

  blochQubitCombo_ = new QComboBox(blochHeader);
  blochQubitCombo_->addItems({"q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7"});
  blochQubitCombo_->setStyleSheet(
      "QComboBox { background-color: #1e293b; color: #f8fafc; border: 1px solid #475569; "
      "border-radius: 4px; padding: 1px 6px; font-size: 11px; min-width: 48px; }");
  blochHeaderLayout->addWidget(blochQubitCombo_);

  QPushButton *resetViewBtn = new QPushButton("Reset View", blochHeader);
  resetViewBtn->setToolTip("Reset 3D camera rotation and zoom");
  resetViewBtn->setStyleSheet(
      "QPushButton { background-color: #334155; color: #e2e8f0; border: 1px solid #475569; "
      "border-radius: 4px; padding: 2px 8px; font-size: 11px; } "
      "QPushButton:hover { background-color: #475569; }");
  blochHeaderLayout->addWidget(resetViewBtn);
  connect(resetViewBtn, &QPushButton::clicked, blochSphereWidget, &BlochSphereWidget::resetView);

  blochHeaderLayout->addWidget(createInfoIcon(
      "<b>3D Bloch Sphere & Trajectory</b><br/>"
      "Geometric visualization of single-qubit quantum state evolution.<br/>"
      "• <b>Numbered Markers</b>: Quantum state waypoints at each gate.<br/>"
      "• <b>Gold Trail</b>: Executed quantum trajectory.<br/>"
      "• <b>Dashed Purple Trail</b>: Future planned trajectory.<br/>"
      "• Click any marker on the sphere or use [Run] to animate!"));
  blochDock->setTitleBarWidget(blochHeader);
  blochDock->setWidget(blochSphereWidget);

  // 5. Entanglement & Superposition Dock
  entanglementDock = new QDockWidget("⚡ Entanglement & Superposition", this);
  entanglementDock->setObjectName("entanglementDock");
  QWidget *entHeader = new QWidget();
  QHBoxLayout *entHeaderLayout = new QHBoxLayout(entHeader);
  entHeaderLayout->setContentsMargins(6, 4, 6, 4);
  QLabel *entTitle = new QLabel("⚡ Entanglement & Superposition", entHeader);
  entTitle->setStyleSheet("font-weight: bold; font-size: 12px; color: #00e5ff;");
  entHeaderLayout->addWidget(entTitle);
  entHeaderLayout->addStretch();
  entHeaderLayout->addWidget(createInfoIcon(
      "<b>Quantum Entanglement & Superposition Analyzer</b><br/>"
      "• <b>Bipartite Entanglement</b>: Wootters concurrence C, negativity N, tangle τ, and mutual info I(A:B).<br/>"
      "• <b>Bell Basis Tomography</b>: Overlap with |Φ⁺⟩, |Φ⁻⟩, |Ψ⁺⟩, |Ψ⁻⟩.<br/>"
      "• <b>Network Constellation</b>: Visual laser links between entangled qubits.<br/>"
      "• <b>Superposition Inspector</b>: Per-qubit degree 4·P0·P1, relative phase φ, and active computational basis states."));
  entanglementDock->setTitleBarWidget(entHeader);
  entanglementDock->setWidget(makeScrollableDock(entanglementViewer, this));

  // 6. Output dock
  outputDock = new QDockWidget("Output", this);
  outputDock->setObjectName("outputDock");
  outputConsole_ = new QTextEdit(this);
  outputConsole_->setReadOnly(true);
  outputConsole_->setStyleSheet(
      "background-color: #12151c; color: #d4d4d4; font-family: 'Menlo', 'Consolas', "
      "monospace; font-size: 11px; border: none;");
  outputDock->setWidget(outputConsole_);
  s_outputConsole = outputConsole_;

  // Dock placement:
  // Left: Tabify Circuit Builder & Qubit Details
  addDockWidget(Qt::LeftDockWidgetArea, circuitDock);
  addDockWidget(Qt::LeftDockWidgetArea, qubitDock);
  tabifyDockWidget(circuitDock, qubitDock);
  circuitDock->raise();

  // Right: Tabify State Analyzer, 3D Bloch Sphere, & Entanglement Viewer
  addDockWidget(Qt::RightDockWidgetArea, stateDock);
  addDockWidget(Qt::RightDockWidgetArea, blochDock);
  addDockWidget(Qt::RightDockWidgetArea, entanglementDock);
  tabifyDockWidget(stateDock, blochDock);
  tabifyDockWidget(blochDock, entanglementDock);
  stateDock->raise();

  // Bottom: Output Console
  addDockWidget(Qt::BottomDockWidgetArea, outputDock);

  // Sync Qubit selection across QubitViewer and 3D Bloch Sphere
  connect(blochQubitCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int q) {
            activeBlochQubit_ = q;
            if (qubitViewer) {
              qubitViewer->setSelectedQubit(q);
            }
            updateStateDisplays();
          });

  connect(qubitViewer, &QubitViewer::qubitSelectionChanged, this, [this](int q) {
    activeBlochQubit_ = q;
    if (blochQubitCombo_ && blochQubitCombo_->currentIndex() != q &&
        q >= 0 && q < blochQubitCombo_->count()) {
      blochQubitCombo_->blockSignals(true);
      blochQubitCombo_->setCurrentIndex(q);
      blochQubitCombo_->blockSignals(false);
    }
    updateStateDisplays();
  });

  // Connect 3D Bloch sphere interactive controls and trajectory marker jumps
  connect(blochSphereWidget, &BlochSphereWidget::runRequested, this,
          &MainWindow::runCircuit);
  connect(blochSphereWidget, &BlochSphereWidget::pauseRequested, this,
          &MainWindow::pauseCircuit);
  connect(blochSphereWidget, &BlochSphereWidget::stepForwardRequested, this,
          &MainWindow::stepForward);
  connect(blochSphereWidget, &BlochSphereWidget::resetRequested, this,
          &MainWindow::resetCircuit);
  connect(blochSphereWidget, &BlochSphereWidget::stepSelected, this,
          [this](int step) {
            if (step >= 0 && step <= totalSteps) {
              currentStep = step;
              if (stepSpinBox) {
                stepSpinBox->blockSignals(true);
                stepSpinBox->setValue(currentStep);
                stepSpinBox->blockSignals(false);
              }
              coreInterface->executeStep(currentStep);
              circuitView->setCurrentStep(currentStep);
              updateStateDisplays();
              updateStatus();
              statusBar()->showMessage(
                  QString("Jumped to step %1 via 3D Bloch marker").arg(currentStep));
            }
          });
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

  // View actions
  resetLayoutAction = new QAction("Reset Dock Layout", this);
  connect(resetLayoutAction, &QAction::triggered, this, &MainWindow::resetDockLayout);

  // Help actions
  aboutAction = new QAction("About", this);
  connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::newCircuit() {
  pauseCircuit();
  coreInterface->createCircuit(2);
  currentStep = 0;
  totalSteps = 0;
  syncCircuitWithBackend();
  statusBar()->showMessage("New 2-qubit circuit created");
  logMessage("Created new 2-qubit quantum circuit.");
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
    coreInterface->resetExecution();
    currentStep = 0;
    syncCircuitWithBackend();

    // Apply recommended view mode if present
    QString viewMode = coreInterface->getRecommendedViewMode();
    if (!viewMode.isEmpty() && stateViewer) {
      stateViewer->setViewMode(viewMode);
    }

    statusBar()->showMessage("Loaded circuit: " + fileName);
    logMessage("Loaded circuit: " + fileName);
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
  stateViewer->updateEntanglementMeasures(coreInterface->getEntanglementMeasures());

  // Update Entanglement & Superposition Viewer
  if (entanglementViewer) {
    entanglementViewer->updateView();
  }

  // Update Qubit Viewer for all qubits in circuit
  int numQubits = coreInterface->getNumQubits();
  if (numQubits < 1)
    numQubits = 2;

  for (int q = 0; q < numQubits; ++q) {
    QVector<double> qs = coreInterface->getQubitState(q);
    if (!qs.isEmpty()) {
      qubitViewer->updateQubitInfo(q, qs[0], qs[1], qs[2], qs[4], qs[6], qs[7]);
    }
  }

  // Update 3D Bloch Sphere with the active qubit's state
  QVector<double> activeState = coreInterface->getQubitState(activeBlochQubit_);
  if (!activeState.isEmpty()) {
    std::complex<double> alpha(activeState[2], activeState[3]);
    std::complex<double> beta(activeState[4], activeState[5]);
    blochSphereWidget->setQubitIndex(activeBlochQubit_);
    blochSphereWidget->setQuantumState(alpha, beta);
  }

  // Populate trajectory waypoints & markers on 3D Bloch sphere
  auto traj = coreInterface->getQubitTrajectory(activeBlochQubit_);
  blochSphereWidget->clearTrajectory();
  for (const auto &pt : traj) {
    blochSphereWidget->addTrajectoryPoint(pt.step, pt.gateName, pt.theta,
                                          pt.phi, pt.r);
  }
  blochSphereWidget->setActiveStep(currentStep);
  blochSphereWidget->setRunning(isRunning);
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

  circuitView->clear();
  int numQubits = coreInterface->getStateVectorComplex().isEmpty()
                      ? 2
                      : static_cast<int>(std::round(std::log2(
                            coreInterface->getStateVectorComplex().size())));
  circuitView->setNumQubits(numQubits);

  auto gates = coreInterface->getCircuitGates();
  totalSteps = gates.size();
  currentStep = coreInterface->getCurrentStep();

  for (const auto &g : gates) {
    circuitView->addGate(g.step, g.qubit1, g.qubit2, g.type, g.parameter);
  }

  circuitView->setCurrentStep(currentStep);

  if (stepSpinBox) {
    stepSpinBox->blockSignals(true);
    stepSpinBox->setMaximum(totalSteps);
    stepSpinBox->setValue(currentStep);
    stepSpinBox->blockSignals(false);
  }

  updateStateDisplays();
  updateStatus();
}

void MainWindow::saveCircuit() {
  saveCircuitAs();
}

void MainWindow::saveCircuitAs() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "Save Quantum Circuit", "",
      "JSON Circuit (*.json);;OpenQASM (*.qasm);;All Files (*)");
  if (!fileName.isEmpty()) {
    if (fileName.endsWith(".qasm", Qt::CaseInsensitive)) {
      QFile file(fileName);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << coreInterface->getCircuitQASM();
        file.close();
        statusBar()->showMessage("Circuit saved as: " + fileName);
        logMessage("Circuit exported to OpenQASM: " + fileName);
      }
    } else {
      coreInterface->saveCircuit(fileName);
      statusBar()->showMessage("Circuit saved as: " + fileName);
      logMessage("Circuit saved to: " + fileName);
    }
  }
}

void MainWindow::stepForward() {
  if (currentStep < totalSteps) {
    currentStep++;
    if (stepSpinBox) {
      stepSpinBox->blockSignals(true);
      stepSpinBox->setValue(currentStep);
      stepSpinBox->blockSignals(false);
    }
    coreInterface->executeStep(currentStep);
    circuitView->setCurrentStep(currentStep);
    updateStateDisplays();
    updateStatus();
    statusBar()->showMessage(
        QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
  }
}

void MainWindow::stepBackward() {
  if (currentStep > 0) {
    currentStep--;
    if (stepSpinBox) {
      stepSpinBox->blockSignals(true);
      stepSpinBox->setValue(currentStep);
      stepSpinBox->blockSignals(false);
    }
    coreInterface->executeStep(currentStep);
    circuitView->setCurrentStep(currentStep);
    updateStateDisplays();
    updateStatus();
    statusBar()->showMessage(
        QString("Step %1 of %2").arg(currentStep).arg(totalSteps));
  }
}

void MainWindow::runCircuit() {
  if (currentStep >= totalSteps) {
    resetCircuit();
  }
  isRunning = true;
  if (runAction)
    runAction->setEnabled(false);
  if (pauseAction)
    pauseAction->setEnabled(true);
  if (blochRunBtn_)
    blochRunBtn_->setText("⏸ Pause");
  if (blochSphereWidget)
    blochSphereWidget->setRunning(true);

  int interval = 400;
  if (speedComboBox) {
    QString speed = speedComboBox->currentText();
    if (speed == "Slow")
      interval = 800;
    else if (speed == "Fast")
      interval = 120;
    else
      interval = 400;
  }
  runTimer_->start(interval);
  statusBar()->showMessage("Running circuit...");
}

void MainWindow::pauseCircuit() {
  isRunning = false;
  if (runTimer_)
    runTimer_->stop();
  if (runAction)
    runAction->setEnabled(true);
  if (pauseAction)
    pauseAction->setEnabled(false);
  if (blochRunBtn_)
    blochRunBtn_->setText("▶ Run");
  if (blochSphereWidget)
    blochSphereWidget->setRunning(false);
  statusBar()->showMessage("Circuit paused");
}

void MainWindow::resetCircuit() {
  pauseCircuit();
  currentStep = 0;
  if (stepSpinBox) {
    stepSpinBox->blockSignals(true);
    stepSpinBox->setValue(0);
    stepSpinBox->blockSignals(false);
  }
  coreInterface->resetExecution();
  circuitView->setCurrentStep(0);
  updateStateDisplays();
  updateStatus();
  statusBar()->showMessage("Circuit reset to step 0");
}

void MainWindow::onRunTimerTick() {
  if (currentStep < totalSteps) {
    int nextStep = currentStep + 1;
    if (circuitView && circuitView->hasBreakpoint(nextStep)) {
      pauseCircuit();
      statusBar()->showMessage(
          QString("Paused at breakpoint at step %1").arg(nextStep));
      logMessage(QString("Breakpoint hit at step %1").arg(nextStep));
      return;
    }
    stepForward();
  } else {
    pauseCircuit();
    statusBar()->showMessage("Execution completed");
  }
}

void MainWindow::onGateAdded(const QString &gateType, int qubit,
                             double parameter) {
  if (coreInterface) {
    QVector<int> qubits;
    QVector<double> parameters;
    if (gateType == "CNOT" || gateType == "SWAP" || gateType == "CP") {
      qubits = {qubit, qubit + 1};
    } else {
      qubits = {qubit};
    }
    if (parameter != 0.0) {
      parameters = {parameter};
    }
    coreInterface->addGate(gateType, qubits, parameters);
    syncCircuitWithBackend();
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

void MainWindow::resetDockLayout() {
  addDockWidget(Qt::LeftDockWidgetArea, circuitDock);
  addDockWidget(Qt::LeftDockWidgetArea, qubitDock);
  tabifyDockWidget(circuitDock, qubitDock);
  circuitDock->show();
  qubitDock->show();
  circuitDock->raise();

  addDockWidget(Qt::RightDockWidgetArea, stateDock);
  addDockWidget(Qt::RightDockWidgetArea, blochDock);
  addDockWidget(Qt::RightDockWidgetArea, entanglementDock);
  tabifyDockWidget(stateDock, blochDock);
  tabifyDockWidget(blochDock, entanglementDock);
  stateDock->show();
  blochDock->show();
  entanglementDock->show();
  stateDock->raise();

  addDockWidget(Qt::BottomDockWidgetArea, outputDock);
  outputDock->show();

  resizeDocks({circuitDock, qubitDock}, {340, 340}, Qt::Horizontal);
  resizeDocks({stateDock, blochDock, entanglementDock}, {440, 440, 440}, Qt::Horizontal);
  statusBar()->showMessage("Layout reset to default", 2500);
}

void MainWindow::loadSettings() {
  QSettings settings;
  int layoutVersion = settings.value("layoutVersion", 0).toInt();
  if (layoutVersion < 2) {
    // Reset layout on upgrade to guarantee clean responsive tabified view
    resetDockLayout();
    settings.setValue("layoutVersion", 2);
    return;
  }
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::saveSettings() {
  QSettings settings;
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.setValue("layoutVersion", 2);
}
