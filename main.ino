#include <LiquidCrystal_I2C.h> // Refer to [3]
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // i2c pin
#define MAX 100
#define inflow 3
#define outflow 2
#define LOW 0x0
#define HIGH 0x1
#define buzzer 6 // buzzer
#define relay 8  // "relay -> pump" brown
// hcsr04
#define trig 10 // orange hcsr04
#define echo 9  // yellow hcsr04
// Input & Button Logic
const int nbuttons = 4;
const int inputPins[nbuttons] = {14, 15, 16, 17};
int inputState[nbuttons];
int lastInputState[nbuttons] = {LOW, LOW, LOW, LOW};
bool inputFlags[nbuttons] = {LOW, LOW, LOW, LOW};
long lastDebounceTime[nbuttons] = {0, 0, 0, 0};
long debounceDelay = 5;
// LCD Menu Logic
const int numOfScreens = 5;
int currentScreen = 0;
String screens[numOfScreens][2] = {{"Max Level", "percent"}, {"Min Level", "percent"}, {"Leak vol/minute", "Litres"}, {"Tank Height", "cm"}, {"Confirm", "YES(1)/NO(0)"}};
int parameters[numOfScreens];
// lcd
//  max and min water level specified in terms of percentages
int flagLevel = 0;
int prevrelaycondition = LOW, relaycondition;
// levels(cm)
float maxlvl = 45, minlvl = 0, tankheight = 25, offset = 0, duration = 0, level = 0,
      distWaterToSensor = 5;
// if leak volume > leakrate, then ring buzzer (leak warning)(l/min)
int leakrate = 5;
void setup()
{
    // pinMode config
    for (int i = 0; i < nbuttons; i++)
    {
        pinMode(inputPins[i], INPUT);
        digitalWrite(inputPins[i], HIGH); // pull-up 20k
    }
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    pinMode(relay, OUTPUT);
    pinMode(buzzer, OUTPUT);
    // serial monitor config, baud rate = 9600
    Serial.begin(9600); // start serial monitoring; set baud rate at 9600 -
    // serial port is capable of transferring a maximum of
    // 9600 bits per second.
    // lcd start
    lcd.begin(16, 2);
    lcd.backlight(); // turns LCD backlight on
    // initialize flow meters
    initflows(3); // initial code for setting up flow meters 1 and 2
    initflows(2);
}
void loop()
{
    if (parameters[4] == 0)
    {
        setInputFlags();
        resolveInputFlags();
    }
    else
    {
        maxlvl = parameters[0];
        minlvl = parameters[1];
        leakrate = parameters[2];
        tankheight = parameters[3];
        Serial.print("working... \n");
        Serial.print("maxlvl");
        Serial.print(maxlvl);
        flows(inflow);
        /*
        digitalWrite() -
        if the pin is configured as an OUTPUT pin,
        writes a HIGH(1) or LOW(0) to a digital pin,
        and by extension the device's pins to which it is conneced to.
        */
        // Manually Write a 10uS pulse to the Trigger Pin
        digitalWrite(trig, LOW);
        delayMicroseconds(2);
        digitalWrite(trig, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig, LOW);

        /* Measure the response from (See how long it stays HIGH == duration)
        echo; PulseIn measures the duration of the response
        */
        duration = pulseIn(echo, HIGH);
        // Calculating distance, formula: uS(duration)/58 cm or range = HIGH signal
        // * velocity (343m/s)/2
        distWaterToSensor = duration / 58;
        distWaterToSensor -= offset;
        // offset correction of 5cm, sensor placed 5cm above max water
        // level; offset for reducing sensor's exposure to water
        Serial.print("Distance Water to Sensor \n");
        Serial.print(distWaterToSensor);
        level = ((tankheight - distWaterToSensor) / (tankheight)) * 100;
        // water level in terms of percentage
        Serial.print("Current level: ");
        Serial.print(level);
        Serial.print("\n");
        delay(1000);
        // function describes relay flow control logic for automating pump action
        Level();
        flows(outflow);
    }
}
void leakwarn(int flowRate)
{
    if (leakrate <= flowRate)
    {
        Serial.write("LEAK DETECTED\n");
        int relaycondition = relayOff();
        Buzzer();
    }
    return relaycondition;
}
void Level()
{
    // water inflow for the first time.
    if (level == 0 || level < minlvl)
    {
        if (prevrelaycondition == LOW)
        {
            relaycondition = relayOn();
        }
        flagLevel = 1;
        Serial.write("Water below minlvl\n");
    }
    else if (flagLevel == 1 && level <= minlvl && level >= maxlvl)
    {
        if (prevrelaycondition == LOW)
        {
            relaycondition = relayOn();
        }
        if ((maxlvl - level) == 0.5)
        {
            // if there is 0.5% of water remaining for waterlevel to reach
            // maxlvl then set flag to zero and initiate next if condition, ie
            // turning off the pump pump stays off until the first if runs.
            flagLevel == 0;
        }
        Serial.write("Water between min and max lvl and rising\n");
    }
    else if (flagLevel == 0 && level >= minlvl && level < maxlvl)
    {
        if (prevrelaycondition == HIGH)
        {
            relaycondition = relayOff();
        }
        Serial.write("Water between min and max lvl and falling\n");
    }
    else if (level >= maxlvl && level <= MAX)
    {
        if (prevrelaycondition == HIGH)
        {
            relaycondition = relayOff();
        }
        Serial.write("Water above max lvl and max height of the tank\n");
    }
    else
    {
        if (prevrelaycondition == HIGH)
            relaycondition = relayOff();
        Serial.write("Water approaching tank limits!\n");
    }
    Serial.write("Relay: ");
    Serial.write((int)relaycondition);
    prevrelaycondition = relaycondition;
}
void Buzzer()
{
    int condition = 0;
    digitalWrite(buzzer, HIGH);
    delay(3000);
    digitalWrite(buzzer, LOW);
}
float inflows;
void dryRunWarn(int flowRate)
{
    Serial.write("LEAK DETECTED\n");
    if (inflows == 0 && relaycondition == HIGH)
    {
        delay(50); // considering time for relay to switch on and/or any
        // obstacles in path
        relayOff();
        Buzzer();
    }
}
int relayOn()
{
    // turn on pump
    digitalWrite(relay, HIGH);
    Serial.write("PUMP ON\n");
    return HIGH;
}
int relayOff()
{
    digitalWrite(relay, LOW);
    Serial.write("PUMP OFF\n");
    return LOW;
}
typedef struct
{
    volatile byte pulseCount;
    float flowRate;
    unsigned int flowMilliLitres;
    unsigned long totalMilliLitres;
    unsigned long oldTime;
    byte pin1;
    byte intr;
} pinx;
pinx pins[4];
int pinflag;
void initflows(int pin)
{
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
    pinflag = pin;
    pins[pin].pulseCount = 0;
    pins[pin].flowRate = 0.0;
    pins[pin].flowMilliLitres = 0;
    pins[pin].totalMilliLitres = 0;
    pins[pin].oldTime = 0;
    if (pin == 2)
    {
        pins[2].pin1 = 2;
        pins[2].intr = 0;
    }
    else
    {
        pins[3].pin1 = 3;
        pins[3].intr = 1;
    }
    attachInterrupt(pins[pin].intr, pulseCounter, FALLING);
}
// 0 = digital pin 2
// pulses per litre 5880;
float calibrationFactor = 60 / 5880;
void flows(int pin)
{
    if ((millis() - pins[pin].oldTime) > 1000) // Only process counters once per second
    {
        // use global pin flag value to increment pulsecount in ISR function
        pinflag = pin;
        // Disable the interrupt while calculating flow rate and sending the
        // value to the host
        detachInterrupt(pins[pin].intr);
        // Because this loop may not complete in exactly 1 second intervals we
        // calculate the number of milliseconds that have passed since the last
        // execution and use that to scale the output. We also apply the
        // calibrationFactor to scale the output based on the number of pulses
        // per second per units of measure (litres/minute in this case) coming
        // from the sensor.
        pins[pin].flowRate = ((1000.0 / (millis() - pins[pin].oldTime)) * pins[pin].pulseCount) / calibrationFactor;
        // Note the time this processing pass was executed. Note that because
        // we've disabled interrupts the millis() function won't actually be
        // incrementing right at this point, but it will still return the value
        // it was set to just before interrupts went away.
        pins[pin].oldTime = millis();
        // Divide the flow rate in litres/minute by 60 to determine how many
        // litres have passed through the sensor in this 1 second interval, then
        // multiply by 1000 to convert to milliliters.
        pins[pin].flowMilliLitres = (pins[pin].flowRate / 60) * 1000;
        // Add the milliliters passed in this second to the cumulative total
        pins[pin].totalMilliLitres += pins[pin].flowMilliLitres;
        // Print the flow rate for this second in litres / minute
        Serial.print("\nFlow Meter ( ");
        Serial.print(((int)pin) - 1);
        Serial.print(")\nFlow rate: ");
        Serial.print(int(pins[pin].flowRate)); // Print the casted integer of the variable
        Serial.print("L/min");
        Serial.print("\n");
        // Print the cumulative total of litres, going through water meter since initialization
        Serial.print("Output Liquid Quantity: \n");
        Serial.print(pins[pin].totalMilliLitres);
        Serial.println("mL");
        Serial.print(pins[pin].totalMilliLitres / 1000);
        Serial.print("L");
        dryRunWarn(pins[pin].flowRate);
        leakwarn(pins[pin].flowRate);
        // Reset the pulse counter so we can start incrementing again
        pins[pin].pulseCount = 0;
        // Enable the interrupt again now that we've finished sending output
        attachInterrupt(pins[pin].intr, pulseCounter, FALLING);
    }
}
/*
Interrupt Service Routine function
*/
void pulseCounter()
{
    // Increment the pulse counter
    pins[pinflag].pulseCount++;
}
/*
LCD interface + Button Logic
*/
void setInputFlags()
{
    for (int i = 0; i < nbuttons; i++)
    {
        int reading = digitalRead(inputPins[i]);
        if (reading != lastInputState[i])
        {
            lastDebounceTime[i] = millis();
        }
        if ((millis() - lastDebounceTime[i]) > debounceDelay)
        {
            if (reading != inputState[i])
            {
                inputState[i] = reading;
                if (inputState[i] == HIGH)
                {
                    inputFlags[i] = HIGH;
                }
            }
        }
        lastInputState[i] = reading;
    }
}
void resolveInputFlags()
{
    for (int i = 0; i < nbuttons; i++)
    {
        if (inputFlags[i] == HIGH)
        {
            inputAction(i);
            inputFlags[i] = LOW;
            printScreen();
        }
    }
}
void inputAction(int input)
{
    if (input == 0)
    {
        if (currentScreen == 0)
        {
            currentScreen = numOfScreens - 1;
        }
        else
        {
            currentScreen--;
        }
    }
    else if (input == 1)
    {
        if (currentScreen == numOfScreens - 1)
        {
            currentScreen = 0;
        }
        else
        {
            currentScreen++;
        }
    }
    else if (input == 2)
    {
        parameterChange(0);
    }
    else if (input == 3)
    {
        parameterChange(1);
    }
}
void parameterChange(int key)
{
    if (key == 0)
    {
        parameters[currentScreen]++;
    }
    else if (key == 1)
    {
        parameters[currentScreen]--;
    }
}
void printScreen()
{
    lcd.clear();
    lcd.print(screens[currentScreen][0]);
    lcd.setCursor(0, 1);
    lcd.print(parameters[currentScreen]);
    lcd.print(" ");
    lcd.print(screens[currentScreen][1]);
}