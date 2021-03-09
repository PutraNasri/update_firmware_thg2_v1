import RPi.GPIO as GPIO
from datetime import datetime
import logging
import time
import json
import requests
import sys
import time
import os

relay_1 = 15
relay_2 = 18
relay_3 = 23
relay_4 = 24

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(relay_1, GPIO.OUT) # GPIO Assign mode 1
GPIO.setup(relay_2, GPIO.OUT) # GPIO Assign mode 2
GPIO.setup(relay_3, GPIO.OUT) # GPIO Assign mode 3
GPIO.setup(relay_4, GPIO.OUT) # GPIO Assign mode 4
GPIO.output(relay_1, GPIO.LOW) 
GPIO.output(relay_2, GPIO.LOW) 
GPIO.output(relay_3, GPIO.LOW) 
GPIO.output(relay_4, GPIO.LOW) 

id_device = "mcbnew1"

time.sleep(20)

def get_data_mcb():
	print("ambil data di firebase")
	url_get_data_mcb = "https://api-lora-otoridashboard.et.r.appspot.com/get/mcb"
	body_get_data_mcb = {
			"id_device": id_device
			}
	response = requests.post(url_get_data_mcb, json = body_get_data_mcb)
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

		#if yang mana relay bernilai 1 lalu print
		#if yang mana relay bernilai 0 lalu print
		#if yang mana relay bernilai 2 lalu print
		for i in data_json:
			if i == 'relay_1':
				relay = relay_1
			elif i == 'relay_2':
				relay = relay_2
			elif i == 'relay_3':
				relay = relay_3
			elif i == 'relay_4':
				relay = relay_4
			else: pass

			nilai = str(data_json[i])

			if nilai == '0':
				GPIO.output(relay, GPIO.LOW) # on
			elif nilai == '1':
				pass
			elif nilai == '2':
				GPIO.output(relay, GPIO.HIGH) # off
			else:
				continue

		report_status_relay()
	
	else:
		print("error lain dengan respon code = "+str(status_code_get_data_mcb))
		get_data_mcb()

def report_status_relay():
	status_relay_1 = GPIO.input(relay_1)
	status_relay_2 = GPIO.input(relay_2)
	status_relay_3 = GPIO.input(relay_3)
	status_relay_4 = GPIO.input(relay_4)
	print("status relay 1 = ",format(status_relay_1))
	print("status relay 2 = ",format(status_relay_2))
	print("status relay 3 = ",format(status_relay_3))
	print("status relay 4 = ",format(status_relay_4))
	now = datetime.now()
	dt_string = now.strftime("%d/%m/%Y %H:%M:%S")

	url_report_status_relay = "https://api-lora-otoridashboard.et.r.appspot.com/update/statusdashboard"
	body_report_status_relay = {
			 "id_device": id_device,
			 "relay_1": format(status_relay_1),
			 "relay_2": format(status_relay_2),
			 "relay_3": format(status_relay_3),
			 "relay_4": format(status_relay_4),
			 "time": str(dt_string)
			}
			 

	status_code = 0
	while status_code != 200:
		response = requests.post(url_report_status_relay, json = body_report_status_relay)
		try: print(response.json())
		except: print('error network')
		status_code = response.status_code



while True:
	get_data_mcb()
	# print(int(time.time()))
	time.sleep(20)



