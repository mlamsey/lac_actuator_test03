#include "linearactuator.h"
#include "firgelli.h" // contains drivers from Actuonix
#include "QTime"
#include "QCoreApplication" // for delay

bool connected = false;
bool done = false; // control loop bool

int upperPosLimit = 950; // upper extension limit for actuator
int lowerPosLimit = 50; // lower extension limit for actuator
int upperVelLimit = 1000;
int lowerVelLimit = 100;

int loopRefreshRate = 60; // Hz

Firgelli actuator; // Object containing driver interface for linear actuator

// ----- FUNCTIONS ----- //
linearActuator::linearActuator()
{
    // any initial stuff, i.e. memory allocation
}

void linearActuator::startActuatorThread() // confirmation message upon thread initialization
{
    printf("Linear Actuator Thread Initialized.\n");
}

void linearActuator::actuatorConnect() // connects to board
{
    if (connected == false)
    {
        actuator.Open(1);
        printf("Linear Actuator Connected.\n");
        emit actuatorConnected(); // signal

        initializeDefaults();

        connected = true;

        runActuator();
    }
}

void linearActuator::initializeDefaults() // returns board to default settings
{
    sendPacket(commands.set_accuracy,settings.accuracy);
    setVelocity(settings.velocity);
    setPosition(settings.position);
    actuatorSendVelocity(settings.velocity);
}

void linearActuator::runActuator() // main control loop fcn
{
    int refreshRate = (int)(1000/loopRefreshRate); // Hz->ms
    printf("Refresh rate: %ims\n",refreshRate);

    if (refreshRate << 5) // can't be too speedy, 5ms arbitrary minimum
    {
        refreshRate = 5;
    }

    // Initialize loop feedback variables
    int position = 0;

    // Master control loop
    while (done == false)
    {
        position = getPosition();
        actuatorSendPosition(position);
        delay(refreshRate);
    }
}

int linearActuator::getPosition() // fetches position (to send to GUI)
{
    return sendPacket(commands.get_feedback,0);
}

void linearActuator::setPosition(int position) // sets position
{
    sendPacket(commands.set_position,position);
}

void linearActuator::setVelocity(int velocity) // sets velocity
{
    sendPacket(commands.set_speed,velocity);
    actuatorSendVelocity(velocity);
}

void linearActuator::actuatorReceivePosition(int input) // position input from GUI
{
    int position = input;
    if(position > upperPosLimit) // Overextension check
    {
        position = upperPosLimit;
    }
    if (position < lowerPosLimit)  // Underextension check
    {
        position = lowerPosLimit;
    }
    setPosition(position);
}

void linearActuator::actuatorReceiveVelocity(int input) // velocity input from GUI
{
    int velocity = input;
    if (velocity > upperVelLimit) // High speed check
    {
        velocity = upperVelLimit;
    }
    if (velocity < lowerVelLimit)  // Low speed check
    {
        velocity = lowerVelLimit;
    }
    setVelocity(velocity);
}

int linearActuator::sendPacket(int commandCode, int value) // builds packet to send to board
{
    return actuator.WriteCode(commandCode,value);
}

void linearActuator::processFinished() // CALL ME when you finish to close thread.
{
    emit finished(); // signal
}

void linearActuator::delay(int millisecondsToWait) // from internet
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}
