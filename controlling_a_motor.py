"""
 * Credits: www.plusivo.com
 """

#import the libraries
from bottle import route, run, template, request
#we will import the sleep module from the time library
from time import sleep
#we will import the RPi.GPIO library with the name of GPIO
import RPi.GPIO as GPIO
import Adafruit_DHT
import pigpio

#we will set the pin numbering to the GPIO.BOARD numbering
#for more details check the guide attached to this code
GPIO.setmode(GPIO.BOARD)

#name the type of sensor used
type = Adafruit_DHT.DHT11

#declare the pin used by the sensor in GPIO form
dht11Real = 22
dht11 = 25

#declare the pins used to show the room temp
green = 36
yellow = 38
red = 40

#the next variable stores the pin used to control the speed of the motor
motorspeed_pin = 7

#the next two variables store the pins used to control the direction of the motor
DIRA = 11
DIRB = 13

#the variable "delayOn" stores the time (in seconds) for the motor to remain On
delayOn = 3

#the variable "delayOff" stores the time (in seconds) for the motor to remain Off
delayOff = 1.5
delayMedium = 3
delayFast = 0.5
#we will set the pins as output
GPIO.setup(motorspeed_pin, GPIO.OUT)
GPIO.setup(DIRA, GPIO.OUT)
GPIO.setup(DIRB, GPIO.OUT)
GPIO.setup(green, GPIO.OUT)
GPIO.setup(yellow, GPIO.OUT)
GPIO.setup(red, GPIO.OUT)

#set the sensor as INPUT
GPIO.setup(dht11Real, GPIO.IN)

#create an instance of the pigpio library
pi = pigpio.pi()

#define the pin used by the Buzzer
#this is GPIO18, which is pin 12
buzzer = 18

#the motorspeed_pin will be used as an enable pin on the motor driver
pwmPIN = GPIO.PWM(motorspeed_pin, 100)

#we start the pwm instance with a duty cycle of 0
pwmPIN.start(0)

#define a function to stop the motor
def turnOff():
    #this instruction is used to set the speed of the motor to 0 (Off)
    pwmPIN.ChangeDutyCycle(0)
    #in these instructions the state is irrelevant because the speed is 0
    GPIO.output(DIRA, GPIO.LOW)
    GPIO.output(DIRB, GPIO.LOW)
    pi.hardware_PWM(buzzer, 500, 500000)

def turnAtMedium():
    #this instruction is used to set the speed of the motor to 38%
    pwmPIN.ChangeDutyCycle(38)
    #these instructions are used to turn on the motor in one direction
    GPIO.output(DIRA, GPIO.HIGH)
    GPIO.output(DIRB, GPIO.LOW)

def turnFast():
    #this instruction is used to set the speed of the motor to 38%
    pwmPIN.ChangeDutyCycle(75)
    #these instructions are used to turn on the motor in one direction
    GPIO.output(DIRA, GPIO.HIGH)
    GPIO.output(DIRB, GPIO.LOW)

#define the route for the main page
@route('/')
def index():
    #at the '/' route we will return the index.html
    #template that is in the views folder
    return template('index.html')
@route('/refresh')
#define a function to send data to client
def refresh():
    #silence the buzzer
    pi.hardware_PWM(buzzer, 0, 0)

    #make the reading
    humidity, temperature = Adafruit_DHT.read(type, dht11)
    global buffer1
    global buffer2
    buffer1 = temperature
    buffer2 = humidity
    #we will send the new values if they are not null
    #else the previous readings will be send
    if humidity is not None and temperature is not None:
        print('Temperature = {:.1f}  Humidity = {:.1f}' .format(temperature, humidity))
        if temperature > 40:
            GPIO.output(green, GPIO.LOW)
            GPIO.output(yellow, GPIO.LOW)
            GPIO.output(red, GPIO.HIGH)
            turnOff()

        elif temperature > 30:
            GPIO.output(green, GPIO.LOW)
            GPIO.output(yellow, GPIO.HIGH)
            GPIO.output(red, GPIO.LOW)
            turnAtMedium()
        else:
            GPIO.output(green, GPIO.HIGH)
            GPIO.output(yellow, GPIO.LOW)
            GPIO.output(red, GPIO.LOW)
            turnFast()

        #return the values to the client
        return {'temperature':temperature, 'humidity':humidity}
    else:
        #return the values to the client
        return {'temperature':buffer1, 'humidity':buffer2}

run(host = '0.0.0.0', port = '5000')
