//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QUBITVIEWER_H
#define QUBITVIEWER_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QVector>
#include <QMap>

class BlochSphereWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BlochSphereWidget(QWidget *parent = nullptr);
    void setQubitState(double theta, double phi, double alpha, double beta);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void drawBlochSphere(QPainter &painter);
    void drawQubitState(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawMeasurementBasis(QPainter &painter);
    
    double theta_;  // Polar angle
    double phi_;    // Azimuthal angle
    double alpha_;  // |0⟩ amplitude
    double beta_;   // |1⟩ amplitude
    QPoint center_;
    int radius_;
    bool isDragging_;
    QPoint lastMousePos_;
};

class QubitViewer : public QWidget
{
    Q_OBJECT

public:
    explicit QubitViewer(QWidget *parent = nullptr);
    ~QubitViewer();

public slots:
    void updateQubitInfo(int qubit, double prob0, double prob1, 
                        double alpha = 1.0, double beta = 0.0,
                        double theta = 0.0, double phi = 0.0);
    void updateMeasurementResults(int qubit, const QVector<int> &results);
    void updateExpectationValues(int qubit, double x, double y, double z);
    void resetQubitData();

private slots:
    void onQubitSelected(int qubit);
    void onMeasurementBasisChanged(const QString &basis);
    void onMeasureButtonClicked();
    void onResetButtonClicked();
    void onAnimateState();

private:
    void setupUI();
    void updateBlochSphere();
    void updateMeasurementStats();
    void updateExpectationTable();
    void calculateQubitProperties();
    
    // UI Components
    QComboBox *qubitSelector_;
    BlochSphereWidget *blochSphere_;
    QTableWidget *qubitTable_;
    QTableWidget *measurementTable_;
    QTableWidget *expectationTable_;
    
    // Measurement controls
    QComboBox *measurementBasis_;
    QPushButton *measureButton_;
    QPushButton *resetButton_;
    QSpinBox *measurementCount_;
    
    // Statistics
    QLabel *purityLabel_;
    QLabel *entanglementLabel_;
    QProgressBar *coherenceBar_;
    
    // Data storage
    QMap<int, QVector<double>> qubitStates_;  // qubit -> [prob0, prob1, alpha, beta, theta, phi]
    QMap<int, QVector<int>> measurementResults_;  // qubit -> measurement results
    QMap<int, QVector<double>> expectationValues_;  // qubit -> [X, Y, Z]
    
    // Animation
    QTimer *animationTimer_;
    int currentQubit_;
    double animationPhase_;
    
    // Labels
    QLabel *titleLabel_;
    QGroupBox *blochGroup_;
    QGroupBox *measurementGroup_;
    QGroupBox *statisticsGroup_;
};

#endif // QUBITVIEWER_H
