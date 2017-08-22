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

void MainWindow::actuatorFetchPosition(int input)
{
    ui->posStatus->setText(QString::number(input));
}

void MainWindow::actuatorFetchVelocity(int input)
{
    ui->velStatus->setText(QString::number(input));
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
    connect(actuatorWorker,SIGNAL(actuatorSendPosition(int)),this,SLOT(actuatorFetchPosition(int)));

    // Velocity control + feedback
    connect(this,SIGNAL(actuatorPushVelocity(int)),actuatorWorker,SLOT(actuatorReceiveVelocity(int)));
    connect(actuatorWorker,SIGNAL(actuatorSendVelocity(int)),this,SLOT(actuatorFetchVelocity(int)));

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
