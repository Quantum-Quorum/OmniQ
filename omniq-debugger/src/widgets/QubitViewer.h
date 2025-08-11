//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QUBITVIEWER_H
#define QUBITVIEWER_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>

class QubitViewer : public QWidget
{
    Q_OBJECT

public:
    explicit QubitViewer(QWidget *parent = nullptr);
    ~QubitViewer();

public slots:
    void updateQubitInfo(int qubit, double prob0, double prob1);

private:
    QTableWidget *qubitTable_;
    QLabel *titleLabel_;
};

#endif // QUBITVIEWER_H
