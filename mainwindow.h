#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void actuatorConnected();
    void actuatorFetchDT(int input);
    void actuatorFetchPositionRaw(int input);
    void actuatorFetchPositionMetric(float input);
    void actuatorFetchVelocityRaw(int input);
    void actuatorFetchVelocityMetric(float input);
    void actuatorFetchVelocityReal(float input);
    void actuatorFetchOscillate(bool state);

private slots:
    // Buttons!
    void on_connectButton_clicked();

    void on_posButton_clicked();

    void on_velButton_clicked();

    void on_oscillateButton_clicked();

private:
    Ui::MainWindow *ui;
    void initializeActuatorThread();

signals:
    void actuatorConnect();
    void actuatorPushPosition(int input);
    void actuatorPushVelocity(int input);
    void actuatorPushOscillate(int min, int max);
};

#endif // MAINWINDOW_H
