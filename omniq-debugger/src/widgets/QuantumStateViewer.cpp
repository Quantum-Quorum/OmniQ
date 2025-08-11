//
// Created by Goutham Arcot on 17/07/25.
//

#include "QuantumStateViewer.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>

QuantumStateViewer::QuantumStateViewer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    titleLabel_ = new QLabel("Quantum State", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel_);
    
    stateTextEdit_ = new QTextEdit(this);
    stateTextEdit_->setReadOnly(true);
    stateTextEdit_->setMaximumHeight(200);
    layout->addWidget(stateTextEdit_);
    
    setLayout(layout);
}

QuantumStateViewer::~QuantumStateViewer()
{
}

void QuantumStateViewer::updateState(const QString &stateString)
{
    stateTextEdit_->setText(stateString);
}
