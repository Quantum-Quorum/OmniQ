//
// Created for OmniQ Debugger: Entanglement & Superposition Viewer
//

#include "EntanglementViewer.h"
#include <cmath>
#include <iomanip>
#include <sstream>

// ============================================================================
// EntanglementGraphCanvas Implementation
// ============================================================================

EntanglementGraphCanvas::EntanglementGraphCanvas(QWidget *parent)
    : QWidget(parent), numQubits_(2), selectedQA_(0), selectedQB_(1),
      hoveredNode_(-1) {
  setMouseTracking(true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void EntanglementGraphCanvas::setNumQubits(int numQubits) {
  if (numQubits != numQubits_) {
    numQubits_ = std::max(2, numQubits);
    if (selectedQA_ >= numQubits_)
      selectedQA_ = 0;
    if (selectedQB_ >= numQubits_)
      selectedQB_ = std::min(1, numQubits_ - 1);
    update();
  }
}

void EntanglementGraphCanvas::setLinks(
    const QVector<CoreInterface::EntanglementLinkInfo> &links) {
  links_ = links;
  update();
}

void EntanglementGraphCanvas::setSelectedPair(int qA, int qB) {
  selectedQA_ = qA;
  selectedQB_ = qB;
  update();
}

QPointF EntanglementGraphCanvas::getQubitNodePosition(
    int index, const QRectF &bounds) const {
  if (numQubits_ <= 0)
    return bounds.center();

  double radius = std::min(bounds.width(), bounds.height()) * 0.38;
  double angle = 2.0 * M_PI * index / numQubits_ - M_PI_2;
  return QPointF(bounds.center().x() + radius * std::cos(angle),
                 bounds.center().y() + radius * std::sin(angle));
}

int EntanglementGraphCanvas::getNodeAtPoint(const QPointF &pos,
                                           const QRectF &bounds) const {
  for (int i = 0; i < numQubits_; ++i) {
    QPointF np = getQubitNodePosition(i, bounds);
    double dist = std::hypot(pos.x() - np.x(), pos.y() - np.y());
    if (dist <= 18.0)
      return i;
  }
  return -1;
}

void EntanglementGraphCanvas::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRectF bounds = rect().adjusted(10, 10, -10, -10);

  // Background subtle grid/frame
  painter.fillRect(rect(), QColor(14, 18, 25));

  // Draw faint dashed orbit ring
  double ringRadius = std::min(bounds.width(), bounds.height()) * 0.38;
  painter.setPen(QPen(QColor(46, 54, 70), 1, Qt::DashLine));
  painter.setBrush(Qt::NoBrush);
  painter.drawEllipse(bounds.center(), ringRadius, ringRadius);

  // 1. Draw Entanglement Beams
  for (const auto &link : links_) {
    if (link.qubit1 < 0 || link.qubit1 >= numQubits_ || link.qubit2 < 0 ||
        link.qubit2 >= numQubits_)
      continue;

    QPointF p1 = getQubitNodePosition(link.qubit1, bounds);
    QPointF p2 = getQubitNodePosition(link.qubit2, bounds);

    bool isSelected = (link.qubit1 == selectedQA_ && link.qubit2 == selectedQB_) ||
                      (link.qubit1 == selectedQB_ && link.qubit2 == selectedQA_);

    double strength = std::clamp(link.concurrence, 0.0, 1.0);
    int alpha = static_cast<int>(140 + 115 * strength);

    // Glow layer
    QPen glowPen(isSelected ? QColor(255, 215, 0, 70)
                            : QColor(0, 229, 255, 60),
                 isSelected ? 8.0 : (4.0 + 4.0 * strength));
    painter.setPen(glowPen);
    painter.drawLine(p1, p2);

    // Core laser line
    QPen corePen(isSelected ? QColor(255, 215, 0, alpha)
                            : QColor(0, 229, 255, alpha),
                 isSelected ? 3.0 : (1.5 + 2.0 * strength));
    painter.setPen(corePen);
    painter.drawLine(p1, p2);

    // Midpoint badge showing concurrence
    QPointF mid = (p1 + p2) * 0.5;
    QString badgeText = QString::number(strength, 'f', 2);
    painter.setFont(QFont("Monospace", 8, QFont::Bold));
    QFontMetrics fm(painter.font());
    int tw = fm.horizontalAdvance(badgeText) + 8;
    int th = fm.height() + 2;
    QRectF badgeRect(mid.x() - tw / 2.0, mid.y() - th / 2.0, tw, th);

    painter.setPen(isSelected ? QColor(255, 215, 0) : QColor(0, 229, 255));
    painter.setBrush(QColor(10, 15, 22, 230));
    painter.drawRoundedRect(badgeRect, 4, 4);

    painter.setPen(isSelected ? QColor(255, 240, 150) : QColor(220, 250, 255));
    painter.drawText(badgeRect, Qt::AlignCenter, badgeText);
  }

  // 2. Draw Qubit Nodes
  for (int i = 0; i < numQubits_; ++i) {
    QPointF pos = getQubitNodePosition(i, bounds);
    bool isSel = (i == selectedQA_ || i == selectedQB_);
    bool isHov = (i == hoveredNode_);

    // Check if entangled with any other qubit
    bool hasEntanglement = false;
    for (const auto &link : links_) {
      if ((link.qubit1 == i || link.qubit2 == i) && link.concurrence > 0.001) {
        hasEntanglement = true;
        break;
      }
    }

    double radius = isSel ? 16.0 : (isHov ? 15.0 : 13.0);

    // Outer halo
    if (isSel) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(255, 215, 0, 50));
      painter.drawEllipse(pos, radius + 6, radius + 6);
    } else if (hasEntanglement) {
      painter.setPen(Qt::NoPen);
      painter.setBrush(QColor(0, 229, 255, 40));
      painter.drawEllipse(pos, radius + 4, radius + 4);
    }

    // Node fill
    QColor fillColor = isSel ? QColor(40, 36, 15)
                             : (hasEntanglement ? QColor(10, 35, 45)
                                                : QColor(25, 30, 42));
    QColor borderColor = isSel ? QColor(255, 215, 0)
                               : (hasEntanglement ? QColor(0, 229, 255)
                                                  : QColor(75, 85, 105));

    painter.setBrush(fillColor);
    painter.setPen(QPen(borderColor, isSel ? 2.5 : 1.5));
    painter.drawEllipse(pos, radius, radius);

    // Node text
    painter.setFont(QFont("SansSerif", isSel ? 9 : 8, QFont::Bold));
    painter.setPen(isSel ? QColor(255, 235, 130) : QColor(240, 245, 255));
    QString label = QString("q%1").arg(i);
    painter.drawText(QRectF(pos.x() - radius, pos.y() - radius, radius * 2,
                            radius * 2),
                     Qt::AlignCenter, label);
  }
}

void EntanglementGraphCanvas::mousePressEvent(QMouseEvent *event) {
  QRectF bounds = rect().adjusted(10, 10, -10, -10);
  int clickedNode = getNodeAtPoint(event->position(), bounds);
  if (clickedNode >= 0) {
    if (clickedNode == selectedQA_) {
      // Toggle / switch
      return;
    }
    selectedQB_ = selectedQA_;
    selectedQA_ = clickedNode;
    emit pairSelected(selectedQA_, selectedQB_);
    update();
  }
}

void EntanglementGraphCanvas::mouseMoveEvent(QMouseEvent *event) {
  QRectF bounds = rect().adjusted(10, 10, -10, -10);
  int node = getNodeAtPoint(event->position(), bounds);
  if (node != hoveredNode_) {
    hoveredNode_ = node;
    if (hoveredNode_ >= 0) {
      setCursor(Qt::PointingHandCursor);
      QToolTip::showText(
          event->globalPosition().toPoint(),
          QString("Qubit q%1\nClick to select for bipartite analysis.")
              .arg(hoveredNode_),
          this);
    } else {
      unsetCursor();
      QToolTip::hideText();
    }
    update();
  }
}

// ============================================================================
// EntanglementViewer Main Widget Implementation
// ============================================================================

EntanglementViewer::EntanglementViewer(CoreInterface *core, QWidget *parent)
    : QWidget(parent), coreInterface_(core), activeQA_(0), activeQB_(1),
      lastKnownNumQubits_(2) {
  setupUI();
}

void EntanglementViewer::setupUI() {
  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(6, 6, 6, 6);
  rootLayout->setSpacing(6);

  tabWidget_ = new QTabWidget(this);
  tabWidget_->setStyleSheet(
      "QTabWidget::pane { border: 1px solid #2e3440; background-color: #12151c; border-radius: 4px; } "
      "QTabBar::tab { background: #1a1e28; color: #94a3b8; padding: 6px 14px; margin-right: 2px; "
      "border-top-left-radius: 4px; border-top-right-radius: 4px; font-weight: bold; font-size: 11px; } "
      "QTabBar::tab:selected { background: #242b38; color: #00e5ff; border-bottom: 2px solid #00e5ff; } "
      "QTabBar::tab:hover { background: #1e2430; color: #e2e8f0; }");

  setupEntanglementTab();
  setupSuperpositionTab();

  rootLayout->addWidget(tabWidget_);
}

void EntanglementViewer::setupEntanglementTab() {
  QWidget *entWidget = new QWidget(this);
  QVBoxLayout *entLayout = new QVBoxLayout(entWidget);
  entLayout->setContentsMargins(8, 8, 8, 8);
  entLayout->setSpacing(8);

  // 1. Qubit Pair Selection Bar
  QHBoxLayout *selectorLayout = new QHBoxLayout();
  selectorLayout->setSpacing(8);

  QLabel *pairLabel = new QLabel("Select Qubit Pair:", entWidget);
  pairLabel->setStyleSheet("font-weight: bold; color: #e2e8f0; font-size: 11px;");
  selectorLayout->addWidget(pairLabel);

  QLabel *qaLabel = new QLabel("Qubit A:", entWidget);
  qaLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");
  selectorLayout->addWidget(qaLabel);

  qubitACombo_ = new QComboBox(entWidget);
  qubitACombo_->setStyleSheet(
      "QComboBox { background-color: #1e293b; color: #f8fafc; border: 1px solid #475569; "
      "border-radius: 4px; padding: 3px 8px; font-weight: bold; font-size: 11px; min-width: 55px; }");
  selectorLayout->addWidget(qubitACombo_);

  swapQubitsBtn_ = new QPushButton("⇄ Swap", entWidget);
  swapQubitsBtn_->setToolTip("Swap Qubit A and Qubit B");
  swapQubitsBtn_->setStyleSheet(
      "QPushButton { background-color: #334155; color: #e2e8f0; border: 1px solid #475569; "
      "border-radius: 4px; padding: 3px 8px; font-size: 11px; } "
      "QPushButton:hover { background-color: #475569; }");
  selectorLayout->addWidget(swapQubitsBtn_);

  QLabel *qbLabel = new QLabel("Qubit B:", entWidget);
  qbLabel->setStyleSheet("color: #94a3b8; font-size: 11px;");
  selectorLayout->addWidget(qbLabel);

  qubitBCombo_ = new QComboBox(entWidget);
  qubitBCombo_->setStyleSheet(
      "QComboBox { background-color: #1e293b; color: #f8fafc; border: 1px solid #475569; "
      "border-radius: 4px; padding: 3px 8px; font-weight: bold; font-size: 11px; min-width: 55px; }");
  selectorLayout->addWidget(qubitBCombo_);

  selectorLayout->addStretch();
  entLayout->addLayout(selectorLayout);

  // 2. Verdict Banner / Card
  verdictCard_ = new QFrame(entWidget);
  verdictCard_->setFrameShape(QFrame::StyledPanel);
  QVBoxLayout *verdictLayout = new QVBoxLayout(verdictCard_);
  verdictLayout->setContentsMargins(10, 8, 10, 8);
  verdictLayout->setSpacing(4);

  QHBoxLayout *badgeLine = new QHBoxLayout();
  verdictBadge_ = new QLabel("🟢 SEPARABLE (PRODUCT STATE)", verdictCard_);
  verdictBadge_->setStyleSheet(
      "font-weight: bold; font-size: 12px; color: #10b981; letter-spacing: 0.5px;");
  badgeLine->addWidget(verdictBadge_);
  badgeLine->addStretch();

  concurrenceValLabel_ = new QLabel("Concurrence: 0.000", verdictCard_);
  concurrenceValLabel_->setStyleSheet(
      "font-family: 'Monospace'; font-weight: bold; font-size: 11px; color: #e2e8f0;");
  badgeLine->addWidget(concurrenceValLabel_);
  verdictLayout->addLayout(badgeLine);

  verdictTitle_ = new QLabel("Qubits q0 and q1 are completely separable.", verdictCard_);
  verdictTitle_->setStyleSheet("font-size: 11px; font-weight: bold; color: #f8fafc;");
  verdictLayout->addWidget(verdictTitle_);

  verdictDesc_ = new QLabel(
      "State factorizes as |ψ_AB⟩ = |ψ_A⟩ ⊗ |ψ_B⟩. Zero bipartite entanglement.",
      verdictCard_);
  verdictDesc_->setStyleSheet("font-size: 10px; color: #94a3b8;");
  verdictDesc_->setWordWrap(true);
  verdictLayout->addWidget(verdictDesc_);

  // Concurrence progress bar
  concurrenceBar_ = new QProgressBar(verdictCard_);
  concurrenceBar_->setRange(0, 100);
  concurrenceBar_->setValue(0);
  concurrenceBar_->setFixedHeight(10);
  concurrenceBar_->setTextVisible(false);
  concurrenceBar_->setStyleSheet(
      "QProgressBar { background-color: #1a1e28; border: 1px solid #334155; border-radius: 4px; } "
      "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10b981, stop:0.5 #00e5ff, stop:1 #ffd700); border-radius: 3px; }");
  verdictLayout->addWidget(concurrenceBar_);

  entLayout->addWidget(verdictCard_);

  // 3. Middle Section: Entanglement Topology Graph + Bell Basis Breakdown
  QHBoxLayout *middleLayout = new QHBoxLayout();
  middleLayout->setSpacing(8);

  // Left side: Graph Canvas
  QGroupBox *graphGroup = new QGroupBox("Entanglement Network Topology", entWidget);
  graphGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; font-size: 11px; color: #cbd5e1; border: 1px solid #2e3440; "
      "border-radius: 6px; margin-top: 6px; padding-top: 10px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
  QVBoxLayout *graphLayout = new QVBoxLayout(graphGroup);
  graphLayout->setContentsMargins(4, 8, 4, 4);

  graphCanvas_ = new EntanglementGraphCanvas(graphGroup);
  graphCanvas_->setMinimumHeight(170);
  graphLayout->addWidget(graphCanvas_);
  middleLayout->addWidget(graphGroup, 3);

  // Right side: Bell State Overlaps
  QGroupBox *bellGroup = new QGroupBox("Bell Basis Decomposition", entWidget);
  bellGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; font-size: 11px; color: #cbd5e1; border: 1px solid #2e3440; "
      "border-radius: 6px; margin-top: 6px; padding-top: 10px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
  QVBoxLayout *bellLayout = new QVBoxLayout(bellGroup);
  bellLayout->setContentsMargins(8, 8, 8, 8);
  bellLayout->setSpacing(6);

  auto makeBellRow = [this, bellGroup, &bellLayout](
                         const QString &name, const QString &ket,
                         QProgressBar *&bar, QLabel *&val, const QString &color) {
    QHBoxLayout *header = new QHBoxLayout();
    QLabel *lbl = new QLabel(QString("<b>%1</b> <span style='color:#94a3b8; font-size:9px;'>%2</span>")
                                 .arg(name, ket),
                             bellGroup);
    lbl->setStyleSheet("font-size: 10px; color: #e2e8f0;");
    header->addWidget(lbl);
    header->addStretch();
    val = new QLabel("0.0%", bellGroup);
    val->setStyleSheet("font-family:'Monospace'; font-weight:bold; font-size:10px; color:#e2e8f0;");
    header->addWidget(val);
    bellLayout->addLayout(header);

    bar = new QProgressBar(bellGroup);
    bar->setRange(0, 100);
    bar->setValue(0);
    bar->setFixedHeight(7);
    bar->setTextVisible(false);
    bar->setStyleSheet(QString(
        "QProgressBar { background-color: #1a1e28; border: 1px solid #334155; border-radius: 3px; } "
        "QProgressBar::chunk { background-color: %1; border-radius: 2px; }").arg(color));
    bellLayout->addWidget(bar);
  };

  makeBellRow("|Φ⁺⟩", "(|00⟩ + |11⟩)/√2", bellPhiPlusBar_, bellPhiPlusVal_, "#00e5ff");
  makeBellRow("|Φ⁻⟩", "(|00⟩ - |11⟩)/√2", bellPhiMinusBar_, bellPhiMinusVal_, "#38bdf8");
  makeBellRow("|Ψ⁺⟩", "(|01⟩ + |10⟩)/√2", bellPsiPlusBar_, bellPsiPlusVal_, "#f59e0b");
  makeBellRow("|Ψ⁻⟩", "(|01⟩ - |10⟩)/√2", bellPsiMinusBar_, bellPsiMinusVal_, "#ec4899");

  middleLayout->addWidget(bellGroup, 2);
  entLayout->addLayout(middleLayout);

  // 4. Bottom Section: Metrics Table & 4x4 Reduced Density Matrix
  QHBoxLayout *bottomLayout = new QHBoxLayout();
  bottomLayout->setSpacing(8);

  // Metrics Table
  metricsTable_ = new QTableWidget(entWidget);
  metricsTable_->setColumnCount(2);
  metricsTable_->setRowCount(5);
  metricsTable_->setHorizontalHeaderLabels({"Bipartite Measure", "Value"});
  metricsTable_->horizontalHeader()->setStretchLastSection(true);
  metricsTable_->verticalHeader()->setVisible(false);
  metricsTable_->setMaximumHeight(130);
  metricsTable_->setStyleSheet(
      "QTableWidget { background-color: #181b24; color: #e2e8f0; gridline-color: #2e3440; "
      "border: 1px solid #2e3440; font-size: 11px; } "
      "QHeaderView::section { background-color: #1e2430; color: #94a3b8; font-weight: bold; border: 1px solid #2e3440; }");
  bottomLayout->addWidget(metricsTable_, 1);

  // Density Matrix Table
  densityMatrixTable_ = new QTableWidget(entWidget);
  densityMatrixTable_->setColumnCount(4);
  densityMatrixTable_->setRowCount(4);
  densityMatrixTable_->setHorizontalHeaderLabels({"|00⟩", "|01⟩", "|10⟩", "|11⟩"});
  densityMatrixTable_->setVerticalHeaderLabels({"|00⟩", "|01⟩", "|10⟩", "|11⟩"});
  densityMatrixTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  densityMatrixTable_->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  densityMatrixTable_->setMaximumHeight(130);
  densityMatrixTable_->setStyleSheet(
      "QTableWidget { background-color: #181b24; color: #e2e8f0; gridline-color: #2e3440; "
      "border: 1px solid #2e3440; font-size: 10px; font-family: 'Monospace'; } "
      "QHeaderView::section { background-color: #1e2430; color: #94a3b8; font-weight: bold; border: 1px solid #2e3440; }");
  bottomLayout->addWidget(densityMatrixTable_, 1);

  entLayout->addLayout(bottomLayout);

  tabWidget_->addTab(entWidget, "⚡ Bipartite Entanglement & Bell Pairs");

  // Wire signals
  connect(qubitACombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &EntanglementViewer::onQubitSelectionChanged);
  connect(qubitBCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &EntanglementViewer::onQubitSelectionChanged);
  connect(swapQubitsBtn_, &QPushButton::clicked, this,
          &EntanglementViewer::onSwapQubitsClicked);
  connect(graphCanvas_, &EntanglementGraphCanvas::pairSelected, this,
          &EntanglementViewer::onGraphPairSelected);
}

void EntanglementViewer::setupSuperpositionTab() {
  QWidget *supWidget = new QWidget(this);
  QVBoxLayout *supLayout = new QVBoxLayout(supWidget);
  supLayout->setContentsMargins(8, 8, 8, 8);
  supLayout->setSpacing(8);

  // Summary Banner
  globalSuperpositionSummary_ = new QLabel("Analyzing Quantum Superposition...", supWidget);
  globalSuperpositionSummary_->setStyleSheet(
      "background-color: #1a1e28; color: #e2e8f0; border: 1px solid #2e3440; "
      "border-radius: 4px; padding: 6px 10px; font-size: 11px; font-weight: bold;");
  supLayout->addWidget(globalSuperpositionSummary_);

  // Per-Qubit Superposition Table
  QGroupBox *qubitGroup = new QGroupBox("Individual Qubit Superposition Analysis", supWidget);
  qubitGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; font-size: 11px; color: #cbd5e1; border: 1px solid #2e3440; "
      "border-radius: 6px; margin-top: 6px; padding-top: 10px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
  QVBoxLayout *qubitLayout = new QVBoxLayout(qubitGroup);
  qubitLayout->setContentsMargins(6, 8, 6, 6);

  qubitSuperpositionTable_ = new QTableWidget(qubitGroup);
  qubitSuperpositionTable_->setColumnCount(6);
  qubitSuperpositionTable_->setHorizontalHeaderLabels({
      "Qubit", "Superposition Status", "Degree (4·P0·P1)",
      "P(|0⟩)", "P(|1⟩)", "Relative Phase (φ)"});
  qubitSuperpositionTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  qubitSuperpositionTable_->horizontalHeader()->setStretchLastSection(true);
  qubitSuperpositionTable_->verticalHeader()->setVisible(false);
  qubitSuperpositionTable_->setMaximumHeight(150);
  qubitSuperpositionTable_->setStyleSheet(
      "QTableWidget { background-color: #181b24; color: #e2e8f0; gridline-color: #2e3440; "
      "border: 1px solid #2e3440; font-size: 11px; } "
      "QHeaderView::section { background-color: #1e2430; color: #94a3b8; font-weight: bold; border: 1px solid #2e3440; }");
  qubitLayout->addWidget(qubitSuperpositionTable_);
  supLayout->addWidget(qubitGroup);

  // Computational Basis Active Components
  QGroupBox *basisGroup = new QGroupBox("Computational Basis State Decomposition", supWidget);
  basisGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; font-size: 11px; color: #cbd5e1; border: 1px solid #2e3440; "
      "border-radius: 6px; margin-top: 6px; padding-top: 10px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
  QVBoxLayout *basisLayout = new QVBoxLayout(basisGroup);
  basisLayout->setContentsMargins(6, 8, 6, 6);

  basisStateTable_ = new QTableWidget(basisGroup);
  basisStateTable_->setColumnCount(5);
  basisStateTable_->setHorizontalHeaderLabels({
      "Basis Ket |k⟩", "Binary", "Probability", "Amplitude (Re + Im)", "Phase Angle"});
  basisStateTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  basisStateTable_->horizontalHeader()->setStretchLastSection(true);
  basisStateTable_->verticalHeader()->setVisible(false);
  basisStateTable_->setStyleSheet(
      "QTableWidget { background-color: #181b24; color: #e2e8f0; gridline-color: #2e3440; "
      "border: 1px solid #2e3440; font-size: 11px; font-family: 'Monospace'; } "
      "QHeaderView::section { background-color: #1e2430; color: #94a3b8; font-weight: bold; border: 1px solid #2e3440; }");
  basisLayout->addWidget(basisStateTable_);
  supLayout->addWidget(basisGroup);

  tabWidget_->addTab(supWidget, "🌀 Superposition & State Amplitudes");
}

void EntanglementViewer::updateQubitSelectors() {
  if (!coreInterface_)
    return;

  int numQubits = std::max(2, coreInterface_->getNumQubits());
  if (numQubits != lastKnownNumQubits_ || qubitACombo_->count() != numQubits) {
    lastKnownNumQubits_ = numQubits;
    qubitACombo_->blockSignals(true);
    qubitBCombo_->blockSignals(true);

    qubitACombo_->clear();
    qubitBCombo_->clear();
    for (int i = 0; i < numQubits; ++i) {
      QString itemText = QString("q%1").arg(i);
      qubitACombo_->addItem(itemText, i);
      qubitBCombo_->addItem(itemText, i);
    }

    if (activeQA_ >= numQubits)
      activeQA_ = 0;
    if (activeQB_ >= numQubits)
      activeQB_ = std::min(1, numQubits - 1);

    qubitACombo_->setCurrentIndex(activeQA_);
    qubitBCombo_->setCurrentIndex(activeQB_);

    qubitACombo_->blockSignals(false);
    qubitBCombo_->blockSignals(false);
  }

  graphCanvas_->setNumQubits(numQubits);
}

void EntanglementViewer::onQubitSelectionChanged() {
  activeQA_ = qubitACombo_->currentIndex();
  activeQB_ = qubitBCombo_->currentIndex();
  graphCanvas_->setSelectedPair(activeQA_, activeQB_);
  updateEntanglementAnalysis();
}

void EntanglementViewer::onSwapQubitsClicked() {
  int temp = activeQA_;
  activeQA_ = activeQB_;
  activeQB_ = temp;

  qubitACombo_->blockSignals(true);
  qubitBCombo_->blockSignals(true);
  qubitACombo_->setCurrentIndex(activeQA_);
  qubitBCombo_->setCurrentIndex(activeQB_);
  qubitACombo_->blockSignals(false);
  qubitBCombo_->blockSignals(false);

  graphCanvas_->setSelectedPair(activeQA_, activeQB_);
  updateEntanglementAnalysis();
}

void EntanglementViewer::onGraphPairSelected(int qA, int qB) {
  activeQA_ = qA;
  activeQB_ = qB;

  qubitACombo_->blockSignals(true);
  qubitBCombo_->blockSignals(true);
  qubitACombo_->setCurrentIndex(activeQA_);
  qubitBCombo_->setCurrentIndex(activeQB_);
  qubitACombo_->blockSignals(false);
  qubitBCombo_->blockSignals(false);

  updateEntanglementAnalysis();
}

void EntanglementViewer::updateView() {
  if (!coreInterface_)
    return;

  updateQubitSelectors();
  updateEntanglementAnalysis();
  updateSuperpositionAnalysis();
}

void EntanglementViewer::updateEntanglementAnalysis() {
  if (!coreInterface_)
    return;

  // Update All-Qubit Links for the Network Graph
  auto allLinks = coreInterface_->getAllEntanglementLinks(0.001);
  graphCanvas_->setLinks(allLinks);
  graphCanvas_->setSelectedPair(activeQA_, activeQB_);

  if (activeQA_ == activeQB_) {
    verdictCard_->setStyleSheet(
        "QFrame { background-color: #1e2430; border: 1px solid #475569; border-radius: 6px; }");
    verdictBadge_->setText("⚠️ SAME QUBIT SELECTED");
    verdictBadge_->setStyleSheet("font-weight: bold; font-size: 12px; color: #f59e0b;");
    verdictTitle_->setText("Please select two distinct qubits (qA ≠ qB).");
    verdictDesc_->setText("Bipartite entanglement is defined between two distinct subsystems.");
    concurrenceBar_->setValue(0);
    concurrenceValLabel_->setText("C: N/A");
    return;
  }

  // Exact Bipartite Entanglement calculation
  auto ent = coreInterface_->calculateBipartiteEntanglement(activeQA_, activeQB_);

  // Update Verdict Banner
  if (ent.isEntangled) {
    verdictCard_->setStyleSheet(
        "QFrame { background-color: #0b2230; border: 1.5px solid #00e5ff; border-radius: 6px; }");
    verdictBadge_->setText("⚡ ENTANGLED PAIR");
    verdictBadge_->setStyleSheet(
        "font-weight: bold; font-size: 12px; color: #00e5ff; letter-spacing: 0.5px;");

    QString classTitle;
    if (ent.bellPhiPlus > 0.90) {
      classTitle = QString("Maximally Entangled Bell Pair |Φ⁺⟩ (|00⟩ + |11⟩)/√2 (Overlap: %1%)")
                       .arg(QString::number(ent.bellPhiPlus * 100.0, 'f', 1));
    } else if (ent.bellPhiMinus > 0.90) {
      classTitle = QString("Maximally Entangled Bell Pair |Φ⁻⟩ (|00⟩ - |11⟩)/√2 (Overlap: %1%)")
                       .arg(QString::number(ent.bellPhiMinus * 100.0, 'f', 1));
    } else if (ent.bellPsiPlus > 0.90) {
      classTitle = QString("Maximally Entangled Bell Pair |Ψ⁺⟩ (|01⟩ + |10⟩)/√2 (Overlap: %1%)")
                       .arg(QString::number(ent.bellPsiPlus * 100.0, 'f', 1));
    } else if (ent.bellPsiMinus > 0.90) {
      classTitle = QString("Maximally Entangled Bell Pair |Ψ⁻⟩ (|01⟩ - |10⟩)/√2 (Overlap: %1%)")
                       .arg(QString::number(ent.bellPsiMinus * 100.0, 'f', 1));
    } else if (ent.concurrence >= 0.90) {
      classTitle = QString("Near-Maximally Entangled State (Concurrence: %1)")
                       .arg(QString::number(ent.concurrence, 'f', 4));
    } else {
      classTitle = QString("Partially Entangled Non-Separable State (Concurrence: %1)")
                       .arg(QString::number(ent.concurrence, 'f', 4));
    }
    verdictTitle_->setText(classTitle);

    verdictDesc_->setText(
        QString("Non-separable quantum state. Wootters Concurrence = %1, Negativity = %2, "
                "Mutual Information I(A:B) = %3 bits.")
            .arg(QString::number(ent.concurrence, 'f', 4),
                 QString::number(ent.negativity, 'f', 4),
                 QString::number(ent.mutualInformation, 'f', 4)));
  } else {
    verdictCard_->setStyleSheet(
        "QFrame { background-color: #06241a; border: 1.5px solid #10b981; border-radius: 6px; }");
    verdictBadge_->setText("🟢 SEPARABLE (PRODUCT STATE)");
    verdictBadge_->setStyleSheet(
        "font-weight: bold; font-size: 12px; color: #10b981; letter-spacing: 0.5px;");
    verdictTitle_->setText(
        QString("Qubits q%1 and q%2 are completely separable.").arg(activeQA_).arg(activeQB_));
    verdictDesc_->setText(
        "Factorizable into product state |ψ_AB⟩ = |ψ_A⟩ ⊗ |ψ_B⟩. Zero bipartite entanglement.");
  }

  // Concurrence bar & label
  int concPercent = static_cast<int>(std::round(ent.concurrence * 100.0));
  concurrenceBar_->setValue(concPercent);
  concurrenceValLabel_->setText(
      QString("Concurrence: %1 (%2%)")
          .arg(QString::number(ent.concurrence, 'f', 4), QString::number(concPercent)));

  // Bell overlaps
  auto setBellVal = [](QProgressBar *bar, QLabel *lbl, double overlap) {
    int pct = static_cast<int>(std::round(overlap * 100.0));
    bar->setValue(pct);
    lbl->setText(QString("%1%").arg(QString::number(overlap * 100.0, 'f', 1)));
  };
  setBellVal(bellPhiPlusBar_, bellPhiPlusVal_, ent.bellPhiPlus);
  setBellVal(bellPhiMinusBar_, bellPhiMinusVal_, ent.bellPhiMinus);
  setBellVal(bellPsiPlusBar_, bellPsiPlusVal_, ent.bellPsiPlus);
  setBellVal(bellPsiMinusBar_, bellPsiMinusVal_, ent.bellPsiMinus);

  // Metrics Table
  struct MetricItem {
    QString name;
    QString val;
  };
  QVector<MetricItem> metrics = {
      {"Wootters Concurrence (C)", QString::number(ent.concurrence, 'f', 4)},
      {"Tangle (τ = C²)", QString::number(ent.tangle, 'f', 4)},
      {"Peres-Horodecki Negativity (N)", QString::number(ent.negativity, 'f', 4)},
      {"Quantum Mutual Information I(A:B)",
       QString("%1 bits").arg(QString::number(ent.mutualInformation, 'f', 4))},
      {"Bipartite Separability Status",
       ent.isEntangled ? "⚡ Entangled" : "🟢 Separable"}};

  for (int r = 0; r < metrics.size(); ++r) {
    metricsTable_->setItem(r, 0, new QTableWidgetItem(metrics[r].name));
    QTableWidgetItem *valItem = new QTableWidgetItem(metrics[r].val);
    valItem->setTextAlignment(Qt::AlignCenter);
    metricsTable_->setItem(r, 1, valItem);
  }

  // 4x4 Reduced Density Matrix
  Eigen::Matrix4cd rho =
      coreInterface_->getTwoQubitReducedDensityMatrix(activeQA_, activeQB_);
  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 4; ++c) {
      std::complex<double> z = rho(r, c);
      QString text;
      if (std::abs(z.imag()) < 1e-4) {
        text = QString::number(z.real(), 'f', 3);
      } else if (std::abs(z.real()) < 1e-4) {
        text = QString("%1i").arg(QString::number(z.imag(), 'f', 3));
      } else {
        text = QString("%1 %2 %3i")
                   .arg(QString::number(z.real(), 'f', 2),
                        z.imag() >= 0 ? "+" : "-",
                        QString::number(std::abs(z.imag()), 'f', 2));
      }

      QTableWidgetItem *item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);

      double mag = std::abs(z);
      if (mag > 0.01) {
        int bgAlpha = static_cast<int>(std::min(180.0, 20 + 200 * mag));
        item->setBackground(QColor(0, 150, 255, bgAlpha));
        item->setForeground(QColor(255, 255, 255));
      } else {
        item->setForeground(QColor(130, 140, 160));
      }
      densityMatrixTable_->setItem(r, c, item);
    }
  }
}

void EntanglementViewer::updateSuperpositionAnalysis() {
  if (!coreInterface_)
    return;

  // 1. Per-Qubit Superposition
  auto supList = coreInterface_->getQubitSuperpositionList();
  qubitSuperpositionTable_->setRowCount(supList.size());

  int inSuperpositionCount = 0;
  for (int i = 0; i < supList.size(); ++i) {
    const auto &qinfo = supList[i];
    if (qinfo.isSuperposition)
      inSuperpositionCount++;

    // Qubit label
    QTableWidgetItem *qItem = new QTableWidgetItem(QString("q%1").arg(qinfo.qubit));
    qItem->setTextAlignment(Qt::AlignCenter);
    qubitSuperpositionTable_->setItem(i, 0, qItem);

    // Status pill
    QString statusText;
    QColor statusColor;
    if (qinfo.superpositionDegree >= 0.95) {
      statusText = "⚡ Maximal Superposition (|+⟩ / |-⟩)";
      statusColor = QColor(0, 229, 255);
    } else if (qinfo.superpositionDegree >= 0.10) {
      statusText = "⚡ Partial Superposition";
      statusColor = QColor(255, 215, 0);
    } else {
      statusText = qinfo.prob0 > 0.5 ? "● Pure Basis State |0⟩" : "● Pure Basis State |1⟩";
      statusColor = QColor(148, 163, 184);
    }
    QTableWidgetItem *statusItem = new QTableWidgetItem(statusText);
    statusItem->setForeground(statusColor);
    qubitSuperpositionTable_->setItem(i, 1, statusItem);

    // Degree (4*P0*P1)
    int degPct = static_cast<int>(std::round(qinfo.superpositionDegree * 100.0));
    QTableWidgetItem *degItem = new QTableWidgetItem(QString("%1%").arg(degPct));
    degItem->setTextAlignment(Qt::AlignCenter);
    qubitSuperpositionTable_->setItem(i, 2, degItem);

    // P(0)
    QTableWidgetItem *p0Item =
        new QTableWidgetItem(QString("%1%").arg(QString::number(qinfo.prob0 * 100.0, 'f', 1)));
    p0Item->setTextAlignment(Qt::AlignCenter);
    qubitSuperpositionTable_->setItem(i, 3, p0Item);

    // P(1)
    QTableWidgetItem *p1Item =
        new QTableWidgetItem(QString("%1%").arg(QString::number(qinfo.prob1 * 100.0, 'f', 1)));
    p1Item->setTextAlignment(Qt::AlignCenter);
    qubitSuperpositionTable_->setItem(i, 4, p1Item);

    // Relative Phase
    QString phaseStr = qinfo.isSuperposition
                           ? QString("%1°").arg(QString::number(qinfo.relativePhase, 'f', 1))
                           : "—";
    QTableWidgetItem *phaseItem = new QTableWidgetItem(phaseStr);
    phaseItem->setTextAlignment(Qt::AlignCenter);
    qubitSuperpositionTable_->setItem(i, 5, phaseItem);
  }

  // 2. Multi-Qubit Computational Basis Decomposition
  auto basisComponents = coreInterface_->getActiveSuperpositionComponents(1e-4);
  basisStateTable_->setRowCount(basisComponents.size());

  for (int r = 0; r < basisComponents.size(); ++r) {
    const auto &comp = basisComponents[r];

    // Ket
    QTableWidgetItem *ketItem =
        new QTableWidgetItem(QString("|%1⟩").arg(comp.binaryString));
    ketItem->setTextAlignment(Qt::AlignCenter);
    ketItem->setForeground(QColor(0, 229, 255));
    basisStateTable_->setItem(r, 0, ketItem);

    // Binary index
    QTableWidgetItem *binItem = new QTableWidgetItem(QString("#%1").arg(comp.basisIndex));
    binItem->setTextAlignment(Qt::AlignCenter);
    basisStateTable_->setItem(r, 1, binItem);

    // Probability
    QTableWidgetItem *probItem = new QTableWidgetItem(
        QString("%1%").arg(QString::number(comp.probability * 100.0, 'f', 2)));
    probItem->setTextAlignment(Qt::AlignCenter);
    basisStateTable_->setItem(r, 2, probItem);

    // Complex Amplitude
    QString ampStr;
    if (std::abs(comp.amplitude.imag()) < 1e-4) {
      ampStr = QString::number(comp.amplitude.real(), 'f', 4);
    } else {
      ampStr = QString("%1 %2 %3i")
                   .arg(QString::number(comp.amplitude.real(), 'f', 4),
                        comp.amplitude.imag() >= 0 ? "+" : "-",
                        QString::number(std::abs(comp.amplitude.imag()), 'f', 4));
    }
    QTableWidgetItem *ampItem = new QTableWidgetItem(ampStr);
    ampItem->setTextAlignment(Qt::AlignCenter);
    basisStateTable_->setItem(r, 3, ampItem);

    // Phase Angle
    QTableWidgetItem *phaseItem = new QTableWidgetItem(
        QString("%1°").arg(QString::number(comp.phase, 'f', 1)));
    phaseItem->setTextAlignment(Qt::AlignCenter);
    basisStateTable_->setItem(r, 4, phaseItem);
  }

  // Summary message
  globalSuperpositionSummary_->setText(
      QString("System State: %1 active basis components in superposition | "
              "%2 of %3 qubits in active superposition.")
          .arg(QString::number(basisComponents.size()),
               QString::number(inSuperpositionCount),
               QString::number(supList.size())));
}

