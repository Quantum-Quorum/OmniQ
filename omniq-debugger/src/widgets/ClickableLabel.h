//
// Created by Agent on 19/01/26.
//

#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QString>
#include <QWidget>

class ClickableLabel : public QLabel {
  Q_OBJECT

public:
  explicit ClickableLabel(const QString &text, const QString &tooltip,
                          QWidget *parent = nullptr);
  ~ClickableLabel() override = default;

protected:
  void mousePressEvent(QMouseEvent *event) override;

private:
  QString tooltipText_;
};

#endif // CLICKABLELABEL_H
