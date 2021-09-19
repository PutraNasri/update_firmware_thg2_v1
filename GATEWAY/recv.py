
import RPi.GPIO as GPIO
from time import sleep
from datetime import datetime
import string
import serial
import logging
import time
import logging
import json
import requests
import sys
import os
import base64

led_1 = 13
led_2 = 19
led_3 = 6

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

GPIO.setup(led_1, GPIO.OUT)
GPIO.setup(led_2, GPIO.OUT)
GPIO.setup(led_3, GPIO.OUT)

GPIO.output(led_1, GPIO.HIGH)
GPIO.output(led_2, GPIO.HIGH)
GPIO.output(led_3, GPIO.HIGH)

global path,id_device
path="/home/pi/"
id_device_gtw = "xxxxxxxx"

print("loading")
time.sleep(10)
print("start")

lora = serial.Serial(port='/dev/ttyS0', baudrate=9600, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS)
time.sleep(1)
GPIO.output(led_1, GPIO.HIGH)
GPIO.output(led_2, GPIO.LOW)
GPIO.output(led_3, GPIO.LOW)

def dekode(data):
	return base64.b64decode(data).decode('utf-8')

def get_data_gateway():
	print("ambil data di firebase")
	url_get_gateway = "http://api-lora.otoridashboard.id/get/gateway"
	body = {
			"id_device": id_device_gtw
			}
	# response = requests.request("POST", url, json = body)
	response = requests.post(url_get_gateway, json = body)
	try: data_json=response.json()
	except: 
		print('network error')
		return None

	data_delay = data_json['delay']
	data_lang = data_json['lang']
	data_lat = data_json['lat']
	data_name = data_json['name']
	data_owner = data_json['owner']
	data_node = data_json['nodes']
	
	print("respon code = "+str(response.status_code))


	if response.status_code == 200 :
		response.close()
		print("respon asli = "+str(data_json))
		print("respon json delay = "+data_delay)
		print("respon json lang = "+data_lang)
		print("respon json lat = "+data_lat)
		print("respon json name = "+data_name)
		print("respon json owner = "+data_owner)
		print("respon json nodes = "+str(data_node))
		# simpan data nodes ke dalam txt

		if data_node == "null":
			print("data null")

		elif data_node != "null":
			# print("respon json nodes = "+str(data_node))
			with open('list_node.txt','w') as f:
				semua = ''
				for i in data_node:
					semua+=data_node[i]
					semua+='\n'
				f.write(semua)
		#ambil data txt jadikan list
		#buat variabel data_a = 123
		#jika data_a tidak sama dengan data list yang di txt tadi maka print pass
		#jika sama print sama
		# self.bacatxt()

	else:
		response.close()
		print("error")

def push_data(id_node,data,rssi_value):		
	GPIO.output(led_3, GPIO.HIGH) 

	url_push_data = "http://api-lora.otoridashboard.id/push/node"
	
	now = datetime.now()
	dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
	data1 = data.split('@')[1]
	data2 = data.split('@')[2]
	data3 = data.split('@')[3]
	data4 = data.split('@')[4]
	data5 = data.split('@')[5]
	data6 = data.split('@')[6]
	data_push = str(id_node)+"@"+str(dt_string)+"@"+str(data1)+"@"+str(data2)+"@"+str(data3)+"@"+str(data4)+"@"+str(data5)+"@"+str(data6)+"@"+str(rssi_value)

	url_push_data = "http://api-lora.otoridashboard.id/push/node"
	body = {
		"id_device": str(id_node),
		"data": str(data_push)
	}

	print("cek data log")
	with open(path+'log.txt','r') as note:
		notes = [n.strip() for n in note.readlines()]
		if len(notes) == 0:
			print("log kosong")
			pass
			# return None
		else: 
			print('log ada')
			for i in notes:
				param = {
					'id_device': i.split('@')[0],
					'data': i
				}
				push_data_log(url_push_data, param)

	print("send data to firebase")
	response = requests.request("POST", url_push_data, json = body)
	# data_json=response.json()

	if response.status_code == 200 :
		response.close()
		GPIO.output(led_3, GPIO.LOW) 
		print("respon json berhasil push = "+str(response.status_code))
		with open(path+'log.txt','w') as note:
			note.write('')

	else:
		print("error push")
		GPIO.output(led_3, GPIO.LOW) 
		log_data(data_push)
		response.close()

def bacatxt():
	list_txt = None
	with open('list_node.txt') as f:
		list_txt = [i.replace('\n', '') for i in f.readlines()]
	return list_txt
					
def log_data(data_push):
	print(data_push)
	with open(path+'log.txt','a') as note:
		note.write(data_push+'\n')

def push_data_log(url_push_data, param):
	requests.post(url_push_data, json=param)
	print('push data success')


#######get_start_hire########
############################
get_data_gateway()

try:
	while True:
		data = lora.readline()
		serialMsg = data.decode('utf-8','ignore')
		print("data mentah = "+serialMsg)
		serialMsg = dekode(serialMsg)
		print("data decode base64 = "+serialMsg)
		id_node = serialMsg.split('@')[0]
		rssi_value = "100" #not support in lora E32
		GPIO.output(led_2, GPIO.HIGH)
		time.sleep(1)
		GPIO.output(led_2, GPIO.LOW)

		with open(path+'list_node.txt') as f:
			list_txt = f.readlines()

		txt=bacatxt()
		
		if id_node in txt:
			# print("kirim data")
			push_data(id_node,serialMsg,rssi_value)
		else:
			print("pass")

except KeyboardInterrupt:
	GPIO.output(led_1, GPIO.LOW)
	GPIO.output(led_2, GPIO.LOW) 
	GPIO.output(led_3, GPIO.LOW) 
	print ("Exiting Program")
except:
	GPIO.output(led_1, GPIO.LOW)
	GPIO.output(led_2, GPIO.LOW) 
	GPIO.output(led_3, GPIO.LOW) 
	print ("Error, Exiting Program")
finally:
	lora.close()
	pass
