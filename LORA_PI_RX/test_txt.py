import hashlib
import base64


def nulis():
	with open('note.txt','a') as note:
		contoh = '1@2@3'
		note.write(contoh+'\n')
		# misal = '3@2@1'
		# note.write(contoh+'\n'+misal+'\n'+contoh)

def baca():
	with open('note.txt','r') as note:
		return [n.strip() for n in note.readlines()]

def apus():
	with open('note.txt','w') as note:
		if len(notes) > 0:
			note.write('')

def hashing(data):
	# return hashlib.md5((data).encode('utf-8')).hexdigest()
	return str(base64.b64encode((key+'-'+data).encode('ascii')).decode('utf-8'))

def dehash(enc):
	return str(base64.b64decode(enc).decode('utf-8'))

key = 'kunci-rahasia'
'''
alur
nulis dulu ke file
baca file
encrypt data
decrypt data
apus isi file txt

'''

# nulis()
notes = baca()
print(notes)

# for i in notes:
# 	enc = hashing(i)
# 	print('enc',enc)

# 	dec = dehash(enc).replace(key+'-','')
# 	print('dec',dec)

apus()
