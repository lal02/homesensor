#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import sys
import requests
import json

from rgbmatrix import RGBMatrix, RGBMatrixOptions, graphics
from datetime import datetime

matrix = None

def setup():

    try:  
        global options
        options = RGBMatrixOptions()
        options.rows = 64
        options.cols = 64
        options.chain_length = 1
        options.parallel = 1
        options.hardware_mapping = 'regular'
        options.disable_hardware_pulsing = True
        options.brightness = 70

        global font_4x6
        global font_6x10
        global font_6x13
        global font_9x15

        font_4x6 = graphics.Font()
        font_6x10 = graphics.Font()
        font_6x13 = graphics.Font()
        font_9x15 = graphics.Font()
        font_6x13.LoadFont("/home/pi/rpi-rgb-led-matrix/fonts/6x13.bdf")
        font_6x10.LoadFont("/home/pi/rpi-rgb-led-matrix/fonts/6x10.bdf")
        font_4x6.LoadFont("/home/pi/rpi-rgb-led-matrix/fonts/4x6.bdf")
        font_9x15.LoadFont("/home/pi/rpi-rgb-led-matrix/fonts/9x15.bdf")

        global yellow
        global orange
        global silver
        global white
        global cyan
        global blue
        global golden
        global green
        global brown
        global red

        yellow = graphics.Color(255,255,0)
        orange = graphics.Color(255,165,0)
        white = graphics.Color(255,255,255)
        cyan = graphics.Color(0,255,255)
        blue = graphics.Color(0,0,255)
        silver = graphics.Color(192,192,192)
        golden = graphics.Color(255,215,0)
        green = graphics.Color(0,100,0)
        brown = graphics.Color(101,67,33)
        red = graphics.Color(139,0,0)
       
        global matrix
        matrix = RGBMatrix(options = options)
        
        global api_key
        global lat
        global long
        global url
        api_key = "27d559648153cbeaa67757f09501e96c"
        lat = "48.886551891267615"
        long = "11.412577235374926"
        url = f"https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={long}&appid={api_key}&units=metric"
        
    
        print("setup finished")
    except Exception as e:
        print("Fehler im Setup: {e}")
        raise

def display():
    try:
        printTime()
        printDate()
        printWeather()
        printSensorData()
    except Exception as e:
        print("Fehler in der Display-Funktion: {e}")
        raise

def getTime():
    current = time.time()
    output = time.strftime("%H:%M",time.localtime(current))
    return output

def printTime():
    hour = time.localtime().tm_hour
    graphics.DrawText(matrix,font_6x10,0,16,green,getTime() + "Uhr")
    if hour > 21 or hour < 8:
        graphics.DrawText(matrix,font_6x10,54,16,silver,"\u263D")
    else:
        graphics.DrawText(matrix,font_6x10,54,16,orange,"\u263C")

def printDate():
    current = datetime.now()
    output = getWeekDay(current.strftime("%A")) + " " + f"{current.day:02d}" + "." + f"{current.month:02d}" + "." + str(current.year)[2:]
    graphics.DrawText(matrix,font_4x6,0,6,brown,output)

def printWeather():
    response = requests.get(url)
    if response.status_code == 200:
        data = json.loads(response.text)
        temperature = data['main']['temp_max']
        temp_min = str(round(data['main']['temp_min'],1))
        weather = data['weather'][0]['main']
        temp_string = str(round(temperature,1)) + "/" + temp_min  + "°C"
        graphics.DrawText(matrix,font_6x10,0,26,yellow,temp_string)
        windspeed = round(data['wind']['speed']*3.6)
        graphics.DrawText(matrix,font_6x10,0,36,silver,str(windspeed) + "km/h")
        if weather == "Rain":
            graphics.DrawText(matrix,font_9x15,36,37,cyan,"\uFFFF")
            amount = data['rain']['1h']
            graphics.DrawText(matrix,font_4x6,47,37,brown,str(round(amount*10,1)))
        elif weather == "Snow":
            graphics.DrawText(matrix,font_9x15,45,37,white,"\uFFFE")
        elif weather == "Thunderstorm":
            graphics.DrawText(matrix,font_9x15,45,37,golden,"\uFFFD")
    else:
        print(response.text)

def getWeekDay(day):
    if day == "Monday":
        return "Montag"
    elif day == "Tuesday":
        return "Dienstag"
    elif day == "Wednesday":
        return "Mittwoch"
    elif day == "Thursday":
        return "Donnerstag"
    elif day == "Friday":
        return "Freitag"
    elif day == "Saturday":
        return "Samstag"
    elif day == "Sunday":
        return "Sonntag"
    else:
        return "Error"

def getTemperature():
    url = f"http://10.0.10.56:8080/api/mobile/temperature/newest"
    response = requests.get(url)
    if response.status_code == 200:
        data = json.loads(response.text)
        temperature = data['value']
        return temperature
        
def getHumidity():
    url = f"http://10.0.10.56:8080/api/mobile/humidity/newest"
    response = requests.get(url)
    if response.status_code == 200:
        data = json.loads(response.text)
        humidity = data['value']
        return humidity

def printSensorData():
    temp = getTemperature()
    hum = getHumidity()
    graphics.DrawText(matrix,font_6x10,0,46,red,"Temp: ")
    graphics.DrawText(matrix,font_6x10,0,56,cyan,"Hum: ")
    graphics.DrawText(matrix,font_6x10,25,56,cyan,str(hum))
    graphics.DrawText(matrix,font_6x10,30,46,red,str(temp))

def checkBrightness():
    global matrix
    url = f"http://10.0.10.56:8080/api/mobile/light/newest"
    response = requests.get(url)
    if response.status_code == 200:
        data = json.loads(response.text)
        brightness = data['value']
        if brightness <5.0:
            options.brightness = 1
            matrix = RGBMatrix(options=options)
        elif brightness >20.0:
            options.brightness=100
            matrix = RGBMatrix(options = options)


def main():
    try:
        print("Press CTRL-C to stop")
        setup()
        while True:
            matrix.Clear()
            checkBrightness()
            display()
            time.sleep(60)
    except KeyboardInterrupt:
	    sys.exit(0)

if __name__ == "__main__":
    main()
