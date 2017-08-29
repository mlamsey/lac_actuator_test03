#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "linearactuator.h"

bool actuatorConnection = false; // For crash prevention via if() checks

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initializeActuatorThread();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actuatorConnected()
{
    ui->connectionStatus->setText(tr("Connected."));
    actuatorConnection = true;
}

void MainWindow::actuatorFetchDT(int input)
{
    float Hz = 1000.0/input;
    ui->label_Refresh->setText("Refresh Rate: " + QString::number(Hz, 'f', 1) + "Hz");
}

void MainWindow::actuatorFetchPositionRaw(int input)
{
    ui->posStatus->setText(QString::number(input));
}

void MainWindow::actuatorFetchPositionMetric(float input)
{
    ui->posStatusEngr->setText(QString::number(input, 'f', 2) + " mm");
}

void MainWindow::actuatorFetchVelocityRaw(int input)
{
    ui->velStatus->setText(QString::number(input));
}

void MainWindow::actuatorFetchVelocityMetric(float input)
{
    ui->velStatusEngr->setText(QString::number(input, 'f', 2) + "%");
}

void MainWindow::actuatorFetchVelocityReal(float input)
{
    ui->velStatusReal->setText(QString::number(input, 'f', 2) + "mm/s");
}

void MainWindow::actuatorFetchOscillate(bool state)
{
    if (state == true) // is oscillating
    {
        ui->oscillateButton->setText(tr("Stop!"));
    }
    else if (state == false) // is not oscillating
    {
        ui->oscillateButton->setText(tr("Oscillate!"));
    }
}

void MainWindow::initializeActuatorThread()
{
    QThread* actuatorThread = new QThread;
    linearActuator* actuatorWorker = new linearActuator;

    actuatorWorker->moveToThread(actuatorThread);

    //Connect to actuator
    connect(actuatorThread,SIGNAL(started()),actuatorWorker,SLOT(startActuatorThread()));

    //Cleanup connections
    connect(actuatorWorker,SIGNAL(finished()),actuatorThread,SLOT(quit()));
    connect(actuatorWorker,SIGNAL(finished()),actuatorWorker,SLOT(deleteLater()));
    connect(actuatorThread,SIGNAL(finished()),actuatorThread,SLOT(deleteLater()));
    actuatorThread->start();

    // --- Making Connections :) --- //
    // CONNCET GUI FUNCTIONS TO ACTUATOR FUNCTIONS
    // Functions labelled as follows:
    // Main GUI: push or fetch parameter (i.e. actuatorPushPosition to send position)
    // Actuator: send or receive parameter (i.e. actuatorReceivePosition to receive position)
    // Please use this naming convention for consistency. -matt

    // Connect to actuator via usb + return
    connect(this,SIGNAL(actuatorConnect()),actuatorWorker,SLOT(actuatorConnect()));
    connect(actuatorWorker,SIGNAL(actuatorConnected()),this,SLOT(actuatorConnected()));

    // Position control + feedback
    connect(this,SIGNAL(actuatorPushPosition(int)),actuatorWorker,SLOT(actuatorReceivePosition(int)));
    connect(actuatorWorker,SIGNAL(actuatorSendPositionRaw(int)),this,SLOT(actuatorFetchPositionRaw(int)));
    connect(actuatorWorker,SIGNAL(actuatorSendPositionMetric(float)),this,SLOT(actuatorFetchPositionMetric(float)));

    // Velocity control + feedback
    connect(this,SIGNAL(actuatorPushVelocity(int)),actuatorWorker,SLOT(actuatorReceiveVelocity(int)));
    connect(actuatorWorker,SIGNAL(actuatorSendVelocityRaw(int)),this,SLOT(actuatorFetchVelocityRaw(int)));
    connect(actuatorWorker,SIGNAL(actuatorSendVelocityMetric(float)),this,SLOT(actuatorFetchVelocityMetric(float)));
    connect(actuatorWorker,SIGNAL(actuatorSendVelocityReal(float)),this,SLOT(actuatorFetchVelocityReal(float)));

    // Oscillator toggle
    connect(this,SIGNAL(actuatorPushOscillate(int, int)),actuatorWorker,SLOT(actuatorReceiveOscillate(int, int)));
    connect(actuatorWorker,SIGNAL(actuatorSendOscillate(bool)),this,SLOT(actuatorFetchOscillate(bool)));

    // Get loop time
    connect(actuatorWorker,SIGNAL(actuatorSendDT(int)),this,SLOT(actuatorFetchDT(int)));

}

// Button functions
void MainWindow::on_connectButton_clicked()
{
    actuatorConnect();
}

void MainWindow::on_posButton_clicked()
{
    if (actuatorConnection = true)
    {
        actuatorPushPosition(ui->posInput->text().toInt());
    }
}

void MainWindow::on_velButton_clicked()
{
    if (actuatorConnection = true)
    {
        actuatorPushVelocity(ui->velInput->text().toInt());
    }
}

void MainWindow::on_oscillateButton_clicked()
{
    actuatorPushOscillate(ui->oscMin->text().toInt(),ui->oscMax->text().toInt());
}
