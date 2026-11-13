//
// Created for OmniQ Debugger: Entanglement & Superposition Viewer
//

#ifndef ENTANGLEMENTVIEWER_H
#define ENTANGLEMENTVIEWER_H

#include "../CoreInterface.h"
#include <QComboBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QToolTip>
#include <QVBoxLayout>
#include <QWidget>

// Interactive 2D vector canvas rendering all-qubit entanglement topology
class EntanglementGraphCanvas : public QWidget {
  Q_OBJECT

public:
  explicit EntanglementGraphCanvas(QWidget *parent = nullptr);

  void setNumQubits(int numQubits);
  void setLinks(const QVector<CoreInterface::EntanglementLinkInfo> &links);
  void setSelectedPair(int qA, int qB);

  QSize sizeHint() const override { return QSize(320, 240); }
  QSize minimumSizeHint() const override { return QSize(200, 160); }

signals:
  void pairSelected(int qA, int qB);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

private:
  QPointF getQubitNodePosition(int index, const QRectF &bounds) const;
  int getNodeAtPoint(const QPointF &pos, const QRectF &bounds) const;

  int numQubits_;
  int selectedQA_;
  int selectedQB_;
  int hoveredNode_;
  QVector<CoreInterface::EntanglementLinkInfo> links_;
};

// Main Entanglement & Superposition Viewer Widget
class EntanglementViewer : public QWidget {
  Q_OBJECT

public:
  explicit EntanglementViewer(CoreInterface *core, QWidget *parent = nullptr);
  ~EntanglementViewer() override = default;

public slots:
  void updateView();

private slots:
  void onQubitSelectionChanged();
  void onSwapQubitsClicked();
  void onGraphPairSelected(int qA, int qB);

private:
  void setupUI();
  void setupEntanglementTab();
  void setupSuperpositionTab();
  void updateEntanglementAnalysis();
  void updateSuperpositionAnalysis();
  void updateQubitSelectors();

  CoreInterface *coreInterface_;
  int activeQA_;
  int activeQB_;
  int lastKnownNumQubits_;

  // UI Tabs
  QTabWidget *tabWidget_;

  // Entanglement Tab UI
  QComboBox *qubitACombo_;
  QComboBox *qubitBCombo_;
  QPushButton *swapQubitsBtn_;

  QFrame *verdictCard_;
  QLabel *verdictBadge_;
  QLabel *verdictTitle_;
  QLabel *verdictDesc_;

  QProgressBar *concurrenceBar_;
  QLabel *concurrenceValLabel_;

  QProgressBar *bellPhiPlusBar_;
  QProgressBar *bellPhiMinusBar_;
  QProgressBar *bellPsiPlusBar_;
  QProgressBar *bellPsiMinusBar_;
  QLabel *bellPhiPlusVal_;
  QLabel *bellPhiMinusVal_;
  QLabel *bellPsiPlusVal_;
  QLabel *bellPsiMinusVal_;

  QTableWidget *metricsTable_;
  QTableWidget *densityMatrixTable_;
  EntanglementGraphCanvas *graphCanvas_;

  // Superposition Tab UI
  QTableWidget *qubitSuperpositionTable_;
  QTableWidget *basisStateTable_;
  QLabel *globalSuperpositionSummary_;
};

#endif // ENTANGLEMENTVIEWER_H

