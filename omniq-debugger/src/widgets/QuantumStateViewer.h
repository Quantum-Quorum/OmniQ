//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QUANTUMSTATEVIEWER_H
#define QUANTUMSTATEVIEWER_H

#include "BlochSphereWidget.h" // Assuming this might be needed later or for consistency
#include "EntanglementGraphWidget.h"
#include "HilbertSpaceWidget.h"
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <complex>

class DensityMatrixWidget : public QWidget {
  Q_OBJECT

public:
  explicit DensityMatrixWidget(QWidget *parent = nullptr);
  void setDensityMatrix(const QVector<QVector<std::complex<double>>> &matrix);
  void setStateVector(const QVector<std::complex<double>> &state);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  void drawMatrix(QPainter &painter);
  void drawStateVector(QPainter &painter);
  void drawBasisLabels(QPainter &painter);
  QColor getMatrixColor(double value, double maxValue);
  QString formatComplexNumber(const std::complex<double> &z);

  QVector<QVector<std::complex<double>>> densityMatrix_;
  QVector<std::complex<double>> stateVector_;
  bool showDensityMatrix_;
  int selectedRow_;
  int selectedCol_;
  QPoint lastMousePos_;
};

class QuantumStateViewer : public QWidget {
  Q_OBJECT

public:
  explicit QuantumStateViewer(QWidget *parent = nullptr);
  ~QuantumStateViewer();

  QSize sizeHint() const override;

public slots:
  void updateState(const QString &stateString);
  void updateStateVector(const QVector<std::complex<double>> &stateVector);
  void updateDensityMatrix(
      const QVector<QVector<std::complex<double>>> &densityMatrix);
  void updateEntanglementMeasures(const QMap<QString, double> &measures);
  void updateTomographyData(const QVector<double> &tomographyData);

private slots:
  void onViewModeChanged(const QString &mode);
  void onBasisChanged(const QString &basis);
  void onAnimateClicked();

signals:
  void animateToggled(bool active);

private slots:
  void onCalculateEntanglement();
  void onPerformTomography();
  void onExportState();

private:
  void setupUI();
  void updateStateDisplay();
  void updateEntanglementTable();
  void updateTomographyTable();
  void calculateStateProperties();
  QString formatComplexNumber(const std::complex<double> &z);
  QLabel *createInfoIcon(const QString &tooltip);

  // UI Components
  QComboBox *viewModeSelector_;
  QComboBox *basisSelector_;
  QTextEdit *stateTextEdit_;
  DensityMatrixWidget *densityMatrixWidget_;

  // Tables
  QTableWidget *entanglementTable_;
  QTableWidget *tomographyTable_;
  QTableWidget *propertiesTable_;

  // Controls
  QPushButton *entanglementButton_;
  QPushButton *tomographyButton_;
  QPushButton *exportButton_;
  QPushButton *animateButton_;

  // Labels and displays
  QLabel *purityLabel_;
  QLabel *fidelityLabel_;
  QLabel *vonNeumannLabel_;
  QProgressBar *coherenceBar_;
  QProgressBar *entanglementBar_;

  // Data storage
  QString currentStateString_;
  QVector<std::complex<double>> stateVector_;
  QVector<QVector<std::complex<double>>> densityMatrix_;
  QMap<QString, double> entanglementMeasures_;
  QVector<double> tomographyData_;

  // Animation (Now handled by MainWindow)
  // QTimer *animationTimer_;
  // double animationPhase_;
  // bool isAnimating_;

  // Groups
  QGroupBox *stateGroup_;
  QGroupBox *propertiesGroup_;
  QGroupBox *entanglementGroup_;
  QGroupBox *tomographyGroup_;

  // Labels
  QLabel *titleLabel_;

  // Visualization
  class EntanglementGraphWidget *entanglementGraph_;
  class HilbertSpaceWidget *hilbertSpace_;
};

#endif // QUANTUMSTATEVIEWER_H
