//
// Created by Agent on 19/01/26.
//

#include "ClickableLabel.h"
#include <QCursor>
#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>

ClickableLabel::ClickableLabel(const QString &text, const QString &tooltip,
                               QWidget *parent)
    : QLabel(text, parent), tooltipText_(tooltip) {

  setToolTip(tooltip);
  setCursor(Qt::PointingHandCursor);

  // Default styling for info icons
  setStyleSheet("QLabel {"
                "  color: #2196F3;"
                "  font-weight: bold;"
                "  font-size: 16px;"
                "  padding: 5px;"
                "}"
                "QLabel:hover {"
                "  color: #1976D2;"
                "}");
}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    qDebug() << "Info icon clicked: " << tooltipText_;
    // Show the tooltip text in a message box
    // Use the window title as the box title if possible, or just "Info"
    QString title = "Information";
    if (parentWidget() && parentWidget()->window()) {
      // Try to get a more context-aware title if needed,
      // but "Information" is usually fine.
    }

    QMessageBox::information(this, title, tooltipText_);
  }

  // Call base class implementation
  QLabel::mousePressEvent(event);
}
