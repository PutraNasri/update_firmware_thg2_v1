import logging
import time
import json
import requests
import sys
import os


def get_data_gateway():
	print("ambil data di firebase")
	url = "http://api-lora.otoridashboard.id/get/node"
	body = {
				"id_device": "xxxxxxxx"
				}
	response = requests.request("POST", url, json = body)
	data_json=response.json()
	
	print("respon code = "+str(response.status_code))


	if response.status_code == 200 :
		response.close()
		print("respon json = "+str(data_json))

	else:
		print("error") 

get_data_gateway()
