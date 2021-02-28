import RPi.GPIO as GPIO
import logging
import time
import json
import requests
import sys
import os

RELAIS_1_GPIO = 18 #or 15

GPIO.setmode(GPIO.BCM) 
GPIO.setwarnings(False)

GPIO.setup(RELAIS_1_GPIO, GPIO.OUT) # GPIO Assign mode

GPIO.output(RELAIS_1_GPIO, GPIO.HIGH) # on
time.sleep(3)
GPIO.output(RELAIS_1_GPIO, GPIO.LOW) # out
	
id_device = "mcbnewtest"


def get_data_mcb():
	print("ambil data di firebase")
	url = "http://api-lora.otoridashboard.id/get/mcb"
	body = {
			"id_device": id_device
			}
	# response = requests.request("POST", url, json = body)
	response = requests.post(url, json = body)
	status_code_get_data_mcb = response.status_code
	

	if status_code_get_data_mcb == 200:
		print("respon code = "+str(status_code_get_data_mcb))
		data_json=response.json()
		print("respon JSON = "+str(data_json))
		status_R1= data_json['relay_1']
		status_R2= data_json['relay_2']
		status_R3= data_json['relay_3']
		status_R4= data_json['relay_4']

		print("R1 = "+str(status_R1))
		print("R2 = "+str(status_R2))
		print("R3 = "+str(status_R3))
		print("R4 = "+str(status_R4))

		#pele pele pele
	else:
		print("error lain dengan respon code = "+str(status_code_get_data_mcb))
		get_data_mcb()

get_data_mcb()