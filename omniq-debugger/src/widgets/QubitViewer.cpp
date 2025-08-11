//
// Created by Goutham Arcot on 17/07/25.
//

#include "QubitViewer.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>

QubitViewer::QubitViewer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    titleLabel_ = new QLabel("Qubit Details", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel_);
    
    qubitTable_ = new QTableWidget(this);
    qubitTable_->setColumnCount(3);
    qubitTable_->setHorizontalHeaderLabels({"Qubit", "P(|0⟩)", "P(|1⟩)"});
    qubitTable_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(qubitTable_);
    
    setLayout(layout);
}

QubitViewer::~QubitViewer()
{
}

void QubitViewer::updateQubitInfo(int qubit, double prob0, double prob1)
{
    if (qubit >= qubitTable_->rowCount()) {
        qubitTable_->setRowCount(qubit + 1);
    }
    
    qubitTable_->setItem(qubit, 0, new QTableWidgetItem(QString::number(qubit)));
    qubitTable_->setItem(qubit, 1, new QTableWidgetItem(QString::number(prob0, 'f', 4)));
    qubitTable_->setItem(qubit, 2, new QTableWidgetItem(QString::number(prob1, 'f', 4)));
}
