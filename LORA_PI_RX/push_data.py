import logging
import time
import json
import requests
import sys
import os


def get_data_gateway():
	print("ambil data di firebase")
	url = "http://api-lora.otoridashboard.id/push/node"
	body = {
				"id_device": "xxxxxxxx",
                "data": "xxxxxxxx@23-03-2021 12:12@1@2@3@4@5@6@-122"
				}
	response = requests.request("POST", url, json = body)
	data_json=response.json()
	
	
	if response.status_code == 200 :
		response.close()
		print("respon json = "+str(response.status_code))

	else:
		print("error") 

get_data_gateway()
