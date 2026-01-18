//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QUBITVIEWER_H
#define QUBITVIEWER_H

#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "BlochSphereWidget.h"

class QubitViewer : public QWidget {
  Q_OBJECT

public:
  explicit QubitViewer(QWidget *parent = nullptr);
  ~QubitViewer();

public slots:
  void updateQubitInfo(int qubit, double prob0, double prob1,
                       double alpha = 1.0, double beta = 0.0,
                       double theta = 0.0, double phi = 0.0);
  void updateMeasurementResults(int qubit, const QVector<int> &results);
  void updateExpectationValues(int qubit, double x, double y, double z);
  void resetQubitData();

private slots:
  void onQubitSelected(int qubit);
  void onMeasurementBasisChanged(const QString &basis);
  void onMeasureButtonClicked();
  void onResetButtonClicked();
  void onAnimateState();

private:
  void setupUI();
  void updateBlochSphere();
  void updateMeasurementStats();
  void updateExpectationTable();
  void calculateQubitProperties();

  // UI Components
  QComboBox *qubitSelector_;
  BlochSphereWidget *blochSphere_;
  QTableWidget *qubitTable_;
  QTableWidget *measurementTable_;
  QTableWidget *expectationTable_;

  // Measurement controls
  QComboBox *measurementBasis_;
  QPushButton *measureButton_;
  QPushButton *resetButton_;
  QSpinBox *measurementCount_;

  // Statistics
  QLabel *purityLabel_;
  QLabel *entanglementLabel_;
  QProgressBar *coherenceBar_;

  // Data storage
  QMap<int, QVector<double>>
      qubitStates_; // qubit -> [prob0, prob1, alpha, beta, theta, phi]
  QMap<int, QVector<int>> measurementResults_;   // qubit -> measurement results
  QMap<int, QVector<double>> expectationValues_; // qubit -> [X, Y, Z]

  // Animation
  QTimer *animationTimer_;
  int currentQubit_;
  double animationPhase_;

  // Labels
  QLabel *titleLabel_;
  QGroupBox *blochGroup_;
  QGroupBox *measurementGroup_;
  QGroupBox *statisticsGroup_;
};

#endif // QUBITVIEWER_H
