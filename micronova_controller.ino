#include <SoftwareSerial.h>
SoftwareSerial StoveSerial(D1, D2);

const char stoveOn[4] = {0x80, 0x21, 0x01, 0xA2};
const char stoveOff[4] = {0x80, 0x21, 0x06, 0xA7};

#define stoveStatus 0x21
//0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)
#define ambTemp 0x01
#define fumeTemp 0x5A
#define stovePower 0x34

uint8_t tempAmbient, tempFumes, powerStove;
uint8_t stoveState = 0, lastStoveVal = 0;
bool ack = false;
uint32_t replyDelay = 200;
char stoveRxData[2];
uint8_t ON_TEMP = 70;

void toggleStove()
{
    if (stoveState == 0)
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOn[i]);
            delayMicroseconds(800);
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            StoveSerial.write(stoveOff[i]);
            delayMicroseconds(800);
        }
    }
    if (stoveState == 0)
    {
        stoveState = 1;
    }
}

void getStoveState()
{
    const char readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stoveStatus);
    delay(replyDelay);
    checkStoveReply();

    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(ambTemp);
    delay(replyDelay);
    checkStoveReply();
}

void getFumeTemp()
{
    const char readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(fumeTemp);
    delay(replyDelay);
    checkStoveReply();
}

void getStovePower()
{
    const char readByte = 0x00;
    StoveSerial.write(readByte);
    delay(1);
    StoveSerial.write(stovePower);
    delay(replyDelay);
    checkStoveReply();
}

void checkStoveReply()
{
    uint8_t rxCount = 0;
    stoveRxData[0] = 0x00;
    stoveRxData[1] = 0x00;
    while (StoveSerial.available())
    {
        stoveRxData[rxCount] = StoveSerial.read();
        //Serial.write(stoveRxData[rxCount]);
        rxCount++;
    }
    if (rxCount == 2)
    {
        byte param = stoveRxData[0] - stoveRxData[1];
        byte val = stoveRxData[1];
        switch (param)
        {
        case stoveStatus:
            stoveState = val;
            Serial.printf("Stove %s\n", stoveState ? "ON" : "OFF");
            break;
        case ambTemp:
            tempAmbient = val / 2;
            Serial.printf("T. amb. %d\n", tempAmbient);
            break;
        case stovePower:
            powerStove = val;
            Serial.printf("Power %d\n", powerStove);
            break;
        case fumeTemp:
            tempFumes = val;
            Serial.printf("T. fumes %d\n", tempFumes);
            break;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    StoveSerial.begin(1200);
    StoveSerial.enableIntTx(false);
    attachInterrupt(digitalPinToInterrupt(D3), toggleStove, FALLING);
}

void loop()
{
    checkStoveReply();
    while (Serial.available())
    {
        StoveSerial.write(Serial.read());
        if (Serial.read() == 'ON')
        {
            for (int i = 0; i < 4; i++)
            {
                StoveSerial.write(stoveOn[i]);
                delayMicroseconds(800);
            }
        }
        else if (Serial.read() == 'OFF')
        {
            for (int i = 0; i < 4; i++)
            {
                StoveSerial.write(stoveOff[i]);
                delayMicroseconds(800);
            }
        }
    }
}
