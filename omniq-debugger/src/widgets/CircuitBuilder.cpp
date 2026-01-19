//
// Created by Goutham Arcot on 17/07/25.
//

#include "CircuitBuilder.h"
#include "ClickableLabel.h"
#include <QApplication>
#include <QDrag>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>

int GateItem::nextId_ = 0;

GateItem::GateItem(const QString &name, const QString &type,
                   QGraphicsItem *parent)
    : QGraphicsItem(parent), gateName_(name), gateType_(type), id_(++nextId_) {
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setAcceptHoverEvents(true);

  if (type == "H")
    color_ = QColor(100, 150, 255);
  else if (type == "X")
    color_ = QColor(255, 100, 100);
  else if (type == "Y")
    color_ = QColor(255, 200, 100);
  else if (type == "Z")
    color_ = QColor(100, 255, 100);
  else if (type == "CNOT")
    color_ = QColor(200, 100, 255);
  else if (type == "SWAP")
    color_ = QColor(255, 150, 150);
  else
    color_ = QColor(150, 150, 150);
}

QRectF GateItem::boundingRect() const { return QRectF(0, 0, 60, 40); }

void GateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget) {
  Q_UNUSED(option)
  Q_UNUSED(widget)

  painter->setRenderHint(QPainter::Antialiasing);

  QRectF rect = boundingRect();

  painter->setBrush(color_);
  painter->setPen(QPen(Qt::black, 2));
  painter->drawRoundedRect(rect, 8, 8);

  painter->setPen(Qt::white);
  painter->setFont(QFont("Arial", 10, QFont::Bold));
  painter->drawText(rect, Qt::AlignCenter, gateName_);
}

void GateItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QDrag *drag = new QDrag(scene()->views().first());
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(gateType_);
    drag->setMimeData(mimeData);
    drag->exec();
  }
  QGraphicsItem::mousePressEvent(event);
}

void GateItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsItem::mouseMoveEvent(event);
}

CircuitBuilder::CircuitBuilder(QWidget *parent)
    : QWidget(parent), numQubits_(4) {
  setupUI();
}

void CircuitBuilder::setupUI() {
  QHBoxLayout *mainLayout = new QHBoxLayout(this);

  setupGatePalette();
  setupCircuitView();
  setupControls();

  mainLayout->addWidget(createGatePaletteGroup());
  mainLayout->addWidget(createCircuitViewGroup());
  mainLayout->addWidget(createControlsGroup());

  mainLayout->setStretch(0, 1);
  mainLayout->setStretch(1, 3);
  mainLayout->setStretch(2, 1);
}

void CircuitBuilder::setupGatePalette() {
  gatePalette_ = new QListWidget(this);
  gatePalette_->setDragEnabled(true);
  gatePalette_->setAcceptDrops(false);
  gatePalette_->setDropIndicatorShown(false);

  QStringList gates = {"H",    "X",  "Y",  "Z",  "CNOT",
                       "SWAP", "RX", "RY", "RZ", "PHASE"};
  for (const QString &gate : gates) {
    QListWidgetItem *item = new QListWidgetItem(gate, gatePalette_);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable |
                   Qt::ItemIsDragEnabled);
  }

  connect(gatePalette_, &QListWidget::itemSelectionChanged, this,
          &CircuitBuilder::onGateSelected);
}

void CircuitBuilder::setupCircuitView() {
  circuitScene_ = new QGraphicsScene(this);
  circuitView_ = new QGraphicsView(circuitScene_);
  circuitView_->setAcceptDrops(true);
  circuitView_->setRenderHint(QPainter::Antialiasing);

  circuitScene_->setSceneRect(0, 0, 400, 300);

  drawQubitLines();
}

void CircuitBuilder::setupControls() {
  qubitSpinBox_ = new QSpinBox(this);
  qubitSpinBox_->setRange(0, numQubits_ - 1);
  qubitSpinBox_->setValue(0);

  gateTypeCombo_ = new QComboBox(this);
  gateTypeCombo_->addItems(
      {"H", "X", "Y", "Z", "CNOT", "SWAP", "RX", "RY", "RZ", "PHASE"});

  parameterSpinBox_ = new QDoubleSpinBox(this);
  parameterSpinBox_->setRange(-3.14159, 3.14159);
  parameterSpinBox_->setValue(0.0);
  parameterSpinBox_->setDecimals(3);

  addGateBtn_ = new QPushButton("Add Gate", this);
  clearBtn_ = new QPushButton("Clear", this);
  loadBtn_ = new QPushButton("Load", this);
  saveBtn_ = new QPushButton("Save", this);

  connect(addGateBtn_, &QPushButton::clicked, this,
          &CircuitBuilder::onAddGateClicked);
  connect(clearBtn_, &QPushButton::clicked, this,
          &CircuitBuilder::onClearClicked);
  connect(loadBtn_, &QPushButton::clicked, this,
          &CircuitBuilder::onLoadClicked);
  connect(saveBtn_, &QPushButton::clicked, this,
          &CircuitBuilder::onSaveClicked);
}

QGroupBox *CircuitBuilder::createGatePaletteGroup() {
  QGroupBox *group = new QGroupBox("Gate Palette", this);
  QVBoxLayout *layout = new QVBoxLayout(group);

  layout->addLayout(createGroupTitleLayout(
      "<b>Gate Palette</b><br/>"
      "Drag and drop interface for instantiating <code>Gate</code> objects. "
      "Available gates correspond to supported operations in the "
      "<code>CoreInterface</code> backend."));

  QLabel *label = new QLabel("Drag gates to circuit:", group);
  layout->addWidget(label);
  layout->addWidget(gatePalette_);

  return group;
}

QGroupBox *CircuitBuilder::createCircuitViewGroup() {
  QGroupBox *group = new QGroupBox("Circuit Builder", this);
  QVBoxLayout *layout = new QVBoxLayout(group);

  layout->addLayout(createGroupTitleLayout(
      "<b>Circuit Canvas</b><br/>"
      "Visual timeline representation of the <code>Circuit</code> model. "
      "Horizontal lines represent qubits (indices 0 to N-1). "
      "Gates are applied sequentially during "
      "<code>CoreInterface::executeFull</code>."));

  layout->addWidget(circuitView_);

  return group;
}

QGroupBox *CircuitBuilder::createControlsGroup() {
  QGroupBox *group = new QGroupBox("Controls", this);
  QVBoxLayout *layout = new QVBoxLayout(group);

  layout->addWidget(new QLabel("Qubit:"));
  layout->addWidget(qubitSpinBox_);

  layout->addWidget(new QLabel("Gate Type:"));
  layout->addWidget(gateTypeCombo_);

  layout->addWidget(new QLabel("Parameter:"));
  layout->addWidget(parameterSpinBox_);

  layout->addWidget(addGateBtn_);
  layout->addWidget(clearBtn_);
  layout->addWidget(loadBtn_);
  layout->addWidget(saveBtn_);

  layout->addStretch();

  return group;
}

void CircuitBuilder::drawQubitLines() {
  circuitScene_->clear();

  for (int i = 0; i < numQubits_; ++i) {
    QGraphicsLineItem *line =
        new QGraphicsLineItem(0, i * 50 + 25, 400, i * 50 + 25);
    line->setPen(QPen(Qt::black, 2));
    circuitScene_->addItem(line);

    QGraphicsTextItem *text = new QGraphicsTextItem(QString("q%1").arg(i));
    text->setPos(-30, i * 50 + 10);
    circuitScene_->addItem(text);
  }
}

void CircuitBuilder::onGateSelected() {
  QListWidgetItem *item = gatePalette_->currentItem();
  if (item) {
    gateTypeCombo_->setCurrentText(item->text());
  }
}

void CircuitBuilder::onAddGateClicked() {
  QString gateType = gateTypeCombo_->currentText();
  int qubit = qubitSpinBox_->value();
  double parameter = parameterSpinBox_->value();

  addGate(gateType, qubit, parameter);
}

void CircuitBuilder::addGate(const QString &gateType, int qubit,
                             double parameter) {
  if (qubit < 0 || qubit >= numQubits_)
    return;

  GateItem *gate = new GateItem(gateType, gateType);
  gate->setPos(placedGates_.size() * 80 + 50, qubit * 50 + 5);
  circuitScene_->addItem(gate);
  placedGates_.append(gate);

  emit gateAdded(gateType, qubit, parameter);
  emit circuitChanged();
}

void CircuitBuilder::clearCircuit() {
  for (GateItem *gate : placedGates_) {
    circuitScene_->removeItem(gate);
    delete gate;
  }
  placedGates_.clear();
  emit circuitChanged();
}

void CircuitBuilder::onClearClicked() { clearCircuit(); }

void CircuitBuilder::onLoadClicked() {
  QString filename = QFileDialog::getOpenFileName(
      this, "Load Circuit", "", "Circuit Files (*.json);;All Files (*)");
  if (!filename.isEmpty()) {
    loadCircuit(filename);
  }
}

void CircuitBuilder::onSaveClicked() {
  QString filename = QFileDialog::getSaveFileName(
      this, "Save Circuit", "", "Circuit Files (*.json);;All Files (*)");
  if (!filename.isEmpty()) {
    saveCircuit(filename);
  }
}

void CircuitBuilder::loadCircuit(const QString &filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, "Error", "Could not open file: " + filename);
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  if (doc.isNull()) {
    QMessageBox::warning(this, "Error", "Invalid JSON file");
    return;
  }

  clearCircuit();

  QJsonObject root = doc.object();
  QJsonArray gates = root["gates"].toArray();

  for (const QJsonValue &value : gates) {
    QJsonObject gate = value.toObject();
    QString type = gate["type"].toString();
    int qubit = gate["qubit"].toInt();
    double parameter = gate["parameter"].toDouble();

    addGate(type, qubit, parameter);
  }
}

void CircuitBuilder::saveCircuit(const QString &filename) {
  QJsonObject root;
  QJsonArray gates;

  for (GateItem *gate : placedGates_) {
    QJsonObject gateObj;
    gateObj["type"] = gate->getType();
    gateObj["qubit"] = static_cast<int>(gate->pos().y() / 50);
    gateObj["parameter"] = 0.0;
    gates.append(gateObj);
  }

  root["gates"] = gates;

  QJsonDocument doc(root);
  QFile file(filename);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson());
  } else {
    QMessageBox::warning(this, "Error", "Could not save file: " + filename);
  }
}

QLabel *CircuitBuilder::createInfoIcon(const QString &tooltip) {
  ClickableLabel *infoIcon = new ClickableLabel("ⓘ", tooltip, this);
  return infoIcon;
}

QHBoxLayout *CircuitBuilder::createGroupTitleLayout(const QString &tooltip) {
  QHBoxLayout *layout = new QHBoxLayout();
  layout->addStretch();
  layout->addWidget(createInfoIcon(tooltip));
  return layout;
}