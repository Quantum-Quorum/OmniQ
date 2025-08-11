//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QUANTUMSTATEVIEWER_H
#define QUANTUMSTATEVIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>

class QuantumStateViewer : public QWidget
{
    Q_OBJECT

public:
    explicit QuantumStateViewer(QWidget *parent = nullptr);
    ~QuantumStateViewer();

public slots:
    void updateState(const QString &stateString);

private:
    QTextEdit *stateTextEdit_;
    QLabel *titleLabel_;
};

#endif // QUANTUMSTATEVIEWER_H
