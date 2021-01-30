import logging
import time
import json
import requests
import sys
import os


def get_data_gateway():
	print("ambil data di firebase")
	url = "http://api-lora.otoridashboard.id/get/gateway"
	body = {
			"id_device": "xxxxxxxx"
			}
	# response = requests.request("POST", url, json = body)
	response = requests.post(url, json = body)
	data_json=response.json()

	data_lang = data_json['lang']
	data_lat = data_json['lat']
	data_name = data_json['name']
	data_owner = data_json['owner']
	data_node = data_json['nodes']
	
	print("respon code = "+str(response.status_code))


	if response.status_code == 200 :
		response.close()
		potong = "id_user@1@2@3@4@5@6"
		data_potong = potong.split('@')[1]
		print("ini data potong = "+data_potong)
		print("respon asli = "+str(data_json))
		print("respon json lang = "+data_lang)
		print("respon json lat = "+data_lat)
		print("respon json name = "+data_name)
		print("respon json owner = "+data_owner)
		if data_node == "null":
			print("data null")

		elif data_node != "null":
			print("respon json nodes = "+str(data_node))
		
		# simpan data nodes ke dalam txt
		
		#ambil data txt jadikan list
		#buat variabel data_a = 123
		#jika data_a tidak sama dengan data list yang di txt tadi maka print pass
		#jika sama print sama
		txt = bacatxt()
		print(txt)
		if 'xxxxx' in txt:
			print("ada")
		else:
			print("pass")

	else:
		print("error")

def bacatxt():
	list_txt = None
	with open('config.txt') as f:
		list_txt = [i.replace('\n', '') for i in f.readlines()]
	return list_txt
	
get_data_gateway()