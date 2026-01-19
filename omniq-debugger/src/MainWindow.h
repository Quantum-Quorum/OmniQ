//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "CoreInterface.h"
#include "widgets/BlochSphereWidget.h"
#include "widgets/CircuitBuilder.h"
#include "widgets/CircuitView.h"
#include "widgets/QuantumStateViewer.h"
#include "widgets/QubitViewer.h"
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void loadCircuit(const QString &fileName);

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
  void onGateAdded(const QString &gateType, int qubit, double parameter);

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
  QDockWidget *circuitDock;
  QDockWidget *stateDock;
  QDockWidget *qubitDock;
  QDockWidget *outputDock;
  QDockWidget *blochDock;

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

  // UI Components
  QSplitter *mainSplitter;
  CircuitView *circuitView;
  QuantumStateViewer *stateViewer;
  QubitViewer *qubitViewer;
  CircuitBuilder *circuitBuilder;
  BlochSphereWidget *blochSphereWidget;

  // ... (rest of UI components)

  // ... (rest of Header)
};

#endif // MAINWINDOW_H
