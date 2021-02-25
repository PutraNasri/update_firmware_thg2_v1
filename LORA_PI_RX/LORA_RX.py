from time import sleep
from datetime import datetime
from SX127x.LoRa import *
from SX127x.board_config import BOARD
import logging
import time
import json
import requests
import sys
import os

global path
path="/home/pi/LORA_PI_RX/"

time.sleep(20)

BOARD.setup()
class LoRaRcvCont(LoRa):
	def __init__(self, verbose=False):
		super(LoRaRcvCont, self).__init__(verbose)
		self.set_mode(MODE.SLEEP)
		self.set_dio_mapping([0] * 6)
	def start(self):
		self.get_data_gateway()
		self.reset_ptr_rx()
		self.set_mode(MODE.RXCONT)
		while True:
			sleep(.5)
			rssi_value = self.get_rssi_value()
			status = self.get_modem_status()
			sys.stdout.flush()         
	def on_rx_done(self):
		# print ("\nReceived: ")
		self.clear_irq_flags(RxDone=1)
		payload = self.read_payload(nocheck=True)
		#print (bytes(payload).decode("utf-8",'ignore'))
		data = bytes(payload).decode("utf-8",'ignore')
		id_node = data.split('@')[0]
		rssi_value = self.get_rssi_value()
		status = self.get_modem_status()
		print ("\nReceived from node = "+data)

		with open(path+'list_node.txt') as f:
			list_txt = f.readlines()

		txt=self.bacatxt()
		if id_node in txt:
			# print("kirim data")
			self.push_data(id_node,data,rssi_value)
		else:
			print("pass")
		
		self.set_mode(MODE.SLEEP)
		self.reset_ptr_rx()
		self.set_mode(MODE.RXCONT) 

	def get_data_gateway(self):
		print("ambil data di firebase")
		url_get_gateway = "http://api-lora.otoridashboard.id/get/gateway"
		body = {
				"id_device": "xxxxxxxx"
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

	def push_data(self,id_node,data,rssi_value):
		
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
					self.push_data_log(url_push_data, param)

		print("send data to firebase")
		response = requests.request("POST", url_push_data, json = body)
		# data_json=response.json()

		if response.status_code == 200 :
			response.close()
			print("respon json berhasil push = "+str(response.status_code))
			with open(path+'log.txt','w') as note:
				note.write('')

		else:
			print("error push")
			self.log_data(data_push)
			response.close()

			
	def bacatxt(self):
		list_txt = None
		with open('list_node.txt') as f:
			list_txt = [i.replace('\n', '') for i in f.readlines()]
		return list_txt

	def log_data(self,data_push):
		print(data_push)
		with open(path+'log.txt','a') as note:
			note.write(data_push+'\n')

	def push_data_log(self,url_push_data, param):
		requests.post(url_push_data, json=param)
		print('push data success')



lora = LoRaRcvCont(verbose=False)
lora.set_mode(MODE.STDBY)
#  Medium Range  Defaults after init are 434.0MHz, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on 13 dBm
lora.set_pa_config(pa_select=1)
try:
	lora.start()
except KeyboardInterrupt:
	sys.stdout.flush()
	print ("")
	sys.stderr.write("KeyboardInterrupt\n")
finally:
	sys.stdout.flush()
	print ("")
	lora.set_mode(MODE.SLEEP)
	BOARD.teardown()