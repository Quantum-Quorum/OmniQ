//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_DEBUGGER_CIRCUITBUILDER_H
#define OMNIQ_DEBUGGER_CIRCUITBUILDER_H

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFrame>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

class GateItem : public QGraphicsItem {
public:
  GateItem(const QString &name, const QString &type,
           QGraphicsItem *parent = nullptr);

  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget = nullptr) override;

  QString getType() const { return gateType_; }
  QString getName() const { return gateName_; }

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
  QString gateName_;
  QString gateType_;
  QColor color_;
  static int nextId_;
  int id_;
};

class CircuitBuilder : public QWidget {
  Q_OBJECT

public:
  explicit CircuitBuilder(QWidget *parent = nullptr);

signals:
  void circuitChanged();
  void gateAdded(const QString &gateType, int qubit, double parameter = 0.0);

public slots:
  void addGate(const QString &gateType, int qubit, double parameter = 0.0);
  void clearCircuit();
  void loadCircuit(const QString &filename);
  void saveCircuit(const QString &filename);

private slots:
  void onGateSelected();
  void onAddGateClicked();
  void onClearClicked();
  void onLoadClicked();
  void onSaveClicked();

private:
  void setupUI();
  void setupGatePalette();
  void setupCircuitView();
  void setupControls();

  QGroupBox *createGatePaletteGroup();
  QGroupBox *createCircuitViewGroup();
  QGroupBox *createControlsGroup();

  QLabel *createInfoIcon(const QString &tooltip);
  QHBoxLayout *createGroupTitleLayout(const QString &tooltip);
  void drawQubitLines();

  QListWidget *gatePalette_;
  QGraphicsView *circuitView_;
  QGraphicsScene *circuitScene_;

  QSpinBox *qubitSpinBox_;
  QComboBox *gateTypeCombo_;
  QDoubleSpinBox *parameterSpinBox_;

  QPushButton *addGateBtn_;
  QPushButton *clearBtn_;
  QPushButton *loadBtn_;
  QPushButton *saveBtn_;

  int numQubits_;
  QList<GateItem *> placedGates_;
};

#endif // OMNIQ_DEBUGGER_CIRCUITBUILDER_H