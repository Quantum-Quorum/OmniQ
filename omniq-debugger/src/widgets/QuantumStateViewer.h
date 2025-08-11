//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QUANTUMSTATEVIEWER_H
#define QUANTUMSTATEVIEWER_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QTableWidget>
#include <QProgressBar>
#include <QPainter>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QStringList>
#include <complex>

class DensityMatrixWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DensityMatrixWidget(QWidget *parent = nullptr);
    void setDensityMatrix(const QVector<QVector<std::complex<double>>> &matrix);
    void setStateVector(const QVector<std::complex<double>> &state);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void drawMatrix(QPainter &painter);
    void drawStateVector(QPainter &painter);
    void drawBasisLabels(QPainter &painter);
    QColor getMatrixColor(double value, double maxValue);
    QString formatComplexNumber(const std::complex<double> &z);
    
    QVector<QVector<std::complex<double>>> densityMatrix_;
    QVector<std::complex<double>> stateVector_;
    bool showDensityMatrix_;
    int selectedRow_;
    int selectedCol_;
    QPoint lastMousePos_;
};

class QuantumStateViewer : public QWidget
{
    Q_OBJECT

public:
    explicit QuantumStateViewer(QWidget *parent = nullptr);
    ~QuantumStateViewer();

public slots:
    void updateState(const QString &stateString);
    void updateStateVector(const QVector<std::complex<double>> &stateVector);
    void updateDensityMatrix(const QVector<QVector<std::complex<double>>> &densityMatrix);
    void updateEntanglementMeasures(const QMap<QString, double> &measures);
    void updateTomographyData(const QVector<double> &tomographyData);

private slots:
    void onViewModeChanged(const QString &mode);
    void onBasisChanged(const QString &basis);
    void onCalculateEntanglement();
    void onPerformTomography();
    void onExportState();
    void onAnimateState();

private:
    void setupUI();
    void updateStateDisplay();
    void updateEntanglementTable();
    void updateTomographyTable();
    void calculateStateProperties();
    QString formatComplexNumber(const std::complex<double> &z);
    
    // UI Components
    QComboBox *viewModeSelector_;
    QComboBox *basisSelector_;
    QTextEdit *stateTextEdit_;
    DensityMatrixWidget *densityMatrixWidget_;
    
    // Tables
    QTableWidget *entanglementTable_;
    QTableWidget *tomographyTable_;
    QTableWidget *propertiesTable_;
    
    // Controls
    QPushButton *entanglementButton_;
    QPushButton *tomographyButton_;
    QPushButton *exportButton_;
    QPushButton *animateButton_;
    
    // Labels and displays
    QLabel *purityLabel_;
    QLabel *fidelityLabel_;
    QLabel *vonNeumannLabel_;
    QProgressBar *coherenceBar_;
    QProgressBar *entanglementBar_;
    
    // Data storage
    QString currentStateString_;
    QVector<std::complex<double>> stateVector_;
    QVector<QVector<std::complex<double>>> densityMatrix_;
    QMap<QString, double> entanglementMeasures_;
    QVector<double> tomographyData_;
    
    // Animation
    QTimer *animationTimer_;
    double animationPhase_;
    bool isAnimating_;
    
    // Groups
    QGroupBox *stateGroup_;
    QGroupBox *propertiesGroup_;
    QGroupBox *entanglementGroup_;
    QGroupBox *tomographyGroup_;
    
    // Labels
    QLabel *titleLabel_;
};

#endif // QUANTUMSTATEVIEWER_H
