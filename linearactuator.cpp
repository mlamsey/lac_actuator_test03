#include "linearactuator.h"
#include "firgelli.h" // contains drivers from Actuonix
#include "QTime"
#include "QCoreApplication" // for delay

bool connected = false;
bool isOscillating = false; // oscillation function
bool isMoving = false; // oscillation stuff
bool maxHit = false; // oscillation stuff
bool done = false; // control loop bool

int upperPosLimit = 950; // upper extension limit for actuator
int lowerPosLimit = 50; // lower extension limit for actuator
int upperVelLimit = 1000;
int lowerVelLimit = 100;

int loopRefreshRate = 60; // Hz

// Default oscillation uses maxed parameters.
int oscMin = lowerPosLimit;
int oscMax = upperPosLimit;

float actuatorStrokeLength = 30; // mm

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
        //actuator.SetDebug(1);
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
    actuatorSendVelocityRaw(settings.velocity);
}

void linearActuator::runActuator() // main control loop fcn
{

    // Initialize loop feedback variables
    int positionRaw = 0;
    float prevPos = 0;
    float thisPos = 0;
    int prevTime = 0;
    int thisTime = 0;
    int dT = 0;
    int velocityReal = 0;

    // Master control loop
    while (done == false)
    {
        // Get time feedback
        prevTime = thisTime;
        thisTime = QDateTime::currentMSecsSinceEpoch();
        dT = thisTime - prevTime;
        actuatorSendDT(dT);

        // Get position feedback
        positionRaw = getPositionRaw();
        prevPos = thisPos;
        thisPos = convertPosToMetric(positionRaw);
        actuatorSendPositionRaw(positionRaw);
        actuatorSendPositionMetric(thisPos);

        velocityReal = (thisPos - prevPos)/dT;
        actuatorSendVelocityReal(velocityReal);

        // Oscillation function
        if (isOscillating == true && isMoving == false)
        {
            if (positionRaw <= oscMax)
            {
                if(maxHit == false)
                {
                    setPosition(oscMax);
                    isMoving = true;
                    maxHit = true;
                }
                else
                {
                    setPosition(oscMin);
                }
            }
            else
            {
                setPosition(oscMin);
                isMoving = true;
            }

            if ((oscMax - positionRaw) <= (int)(settings.accuracy/2))
            {
                isMoving = false;
            }
            if ((positionRaw - oscMin) <= (int)(settings.accuracy/2))
            {
                isMoving = false;
                maxHit = false;
            }
        }
        //delay(100);
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

int linearActuator::getPositionRaw() // fetches position (to send to GUI)
{
    return sendPacket(commands.get_feedback,0);
}

void linearActuator::setPosition(int position) // sets position
{
    sendPacket(commands.set_position,position);
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

    if (isOscillating == true)
    {
        toggleOscillation(oscMin,oscMax);
    }
    setPosition(position);
}

void linearActuator::setVelocity(int velocity) // sets velocity
{
    sendPacket(commands.set_speed,velocity);
    actuatorSendVelocityRaw(velocity);
    actuatorSendVelocityMetric(convertVelToMetric(velocity));
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

void linearActuator::actuatorReceiveOscillate(int min, int max)
{
    toggleOscillation(min,max);
}

void linearActuator::toggleOscillation(int min, int max)
{
    if (isOscillating == false) // toggle to true
    {
        actuatorSendOscillate(true);
        isOscillating = true;
        oscMin = min;
        oscMax = max;
    }
    else if (isOscillating == true) // toggle to false
    {
        actuatorSendOscillate(false);
        isOscillating = false;
    }
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

float linearActuator::convertPosToMetric(int rawValue)
{
    return ((rawValue*actuatorStrokeLength)/1024); // output mm
}

float linearActuator::convertVelToMetric(int rawValue)
{
    return 100.0*rawValue/1024;
}
