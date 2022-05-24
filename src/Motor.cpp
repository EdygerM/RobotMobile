#include "Motor.h"

Motor::Motor(byte pin1, byte pin2, byte pinSleep, Constant::MotorMode mode, PID controller) :  
  previousTime(micros()), 
  previousPos(0), 
  controller(controller),
  minPWM(5),
  maxPWM(255)
{
  this->pin1 = pin1;
  this->pin2 = pin2;
  this->pinSleep = pinSleep;
  this->mode = mode;
  init();
}

Motor::Motor(byte pin1, byte pin2, byte pinSleep, Constant::MotorMode mode, PID controller, byte minPWM, byte maxPWM) : 
  previousTime(micros()), 
  previousPos(0),
  controller(controller)
{
  this->pin1 = pin1;
  this->pin2 = pin2;
  this->pinSleep = pinSleep;
  this->mode = mode;
  this->minPWM = minPWM;
  this->maxPWM = maxPWM;
  init();
}

void Motor::init() 
{
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pinSleep, OUTPUT);
}

void Motor::setSpeed(float speedSetpoint, bool speedTuning, int position, float speed) 
{ 
  float deltaTime = getDeltaTime();
  int deltaPos = getDeltaPosition(position);
  float speedMeasure = getSpeed(deltaPos, deltaTime);
  float speedOutput = controller.getOutput(speedSetpoint, speedMeasure, deltaTime);
  byte speedPWM = getSpeedPWM(speedOutput);

  /*Serial.print("a:");
  Serial.print(speed);
  Serial.print(", ");*/

  if(speedTuning)
    controller.printTuning(speedSetpoint, speedMeasure);

  sleepManagement(speedPWM, speedMeasure);

  setMotor(speedPWM, isForward(speedOutput));
  //setMotor(speedSetpoint, true);
}

void Motor::setSpeedV2(float speedSetpoint, bool speedTuning, int position, float speed) 
{ 
  float speedOutput = controller.getOutput(speedSetpoint, speed, getDeltaTime());
  byte speedPWM = getSpeedPWM(speedOutput);

  if(speedTuning)
    controller.printTuning(speedSetpoint, speed);

  sleepManagement(speedPWM, speed);

  setMotor(speedPWM, isForward(speedOutput));
}

// Compute time variation between now and the last call
float Motor::getDeltaTime() 
{
  unsigned long currentTime = micros();
  unsigned long deltaTime = currentTime - previousTime; 
  previousTime = currentTime;
  
  return (float)deltaTime*Constant::toMicro;
}

// Compute position variation read on the encoder between now and the last call
int Motor::getDeltaPosition(int position) 
{
  int deltaPos = position - previousPos;
  previousPos = position;
  
  return deltaPos;
}

// Compute the speed depending on the variation of position and time
float Motor::getSpeed(float deltaPos, float deltaTime) 
{
  float speed = 0;
  if(deltaTime > 0)
    speed = deltaPos/deltaTime;

  return deltaPos/deltaTime;
}

float Motor::getSpeedPWM(float speed)
{
  float speedPWM = abs(speed)/15;
  
  if(speedPWM > maxPWM)
    speedPWM = maxPWM;
  else if(speedPWM < minPWM)
    speedPWM = 0;

  return speedPWM;
}

void Motor::sleepManagement(byte speedCommand, int speedMeasure) 
{
  if(pinSleep > 0) {
    if(speedCommand <= minPWM && speedMeasure <= (minPWM*15))
      digitalWrite(pinSleep, LOW);
    else
      digitalWrite(pinSleep, HIGH);
  }
}

bool Motor::isForward(float speedOutput) 
{
  bool isForward = true;
  if(speedOutput < 0)
    isForward = false;

  return isForward;
}

void Motor::setMotor(byte speedPWM, bool isForward)
{
  analogWrite(pin1, speedPWM);
  if(isForward)
    digitalWrite(pin2, HIGH);
  else
    digitalWrite(pin2, LOW);
}
