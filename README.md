# Automatic Tank Water Management System

## COMPONENTS USED
1. Arduino UNO (ATmega328P)<br>
2. Bucket (for simulating reservoir)<br>
3. Pump<br>
4. Relays<br>
5. LCD Module (1602A monochrome)<br>
6. Ultrasonic Distance Sensor (HC-SR04)<br>
7. Jumper Wires<br>
8. Breadboard<br>
9. 9V Battery with Connector<br>
10. Pipes<br>
11. Pushbuttons<br>
12. Water Flow Sensor<br>
13. Buzzer<br>

## PROPOSED WORK

There are 3 states in the water level control system:<br>
![image](https://user-images.githubusercontent.com/68748665/222456168-75738a03-f973-44b9-a4ce-846dd39a0a6a.png)<br><br>

1. Initially, the tank is empty. When the system is started, and the ultrasonic sensor reads the depth of the tank, it would start the water pump, letting water into the tank. It would fill the tank up to the predefined upper limit set by the user. <br><br>

2. Under normal usage in residential systems, the tank would eventually continue to deplete water. When the ultrasonic sensor returns a depth that is lower than the predefined lower limit, the tank system would again open the pump, filling water up to the predefined upper limit. <br><br>

3. As the inflow of water persists, the ultrasonic sensor will keep updating the value of the depth, until the microcontroller detects upper limit depth, at which point the pump switches off. <br><br>

4. A buzzer, which would ring on an indication of an empty water reservoir tank, by reading the flow of water, via a water flow sensor. The motor is switched off. Requires user intervention, to refill the reservoir tank. We will refer to this feature as a “dry-run indicator”. <br><br>

5. An option for a buzzer which rings for 3 seconds, which would ring on indication of too high a water flow, by reading the flow of water, via a water flow sensor, before reaching the residential taps. If too large a volume outflows within a certain amount of time into the residential taps it serves as an indication of a leak, which would require user intervention, for the residential piping. We will refer to this feature as a “leak indicator”. <br><br>

## FLOWCHART
![image](https://user-images.githubusercontent.com/68748665/222456976-6abdd05c-175c-4633-beb1-83f3bbdce238.png)

## CIRCUT DIAGRAM
![image](https://user-images.githubusercontent.com/68748665/222457134-b0140e99-9ce9-49b7-99dc-8a1ad207ecdb.png)

## CONCLUSION
The automatic water tank filling system has been constructed successfully. The novel system is suitable to be used in home activity contributing to the decrease of the energy consumption due to the water spills. Moreover, it can help people analyze water consumption. <br>In present days, there are many parts on earth which face scarcity of water, calamities like drought etc. Energy production is laborious and cannot be misused. The water tank overflows as the height of water in the tank cannot be randomly guessed. This leads to extra energy consumption, which is a high concern in the present era. People also need to wait and stop doing their other activities until the tank is full. Hence, we put forth an implementation which senses and indicates the water level so that the pump can be switched off at appropriate time and save water, electricity and time as well.
