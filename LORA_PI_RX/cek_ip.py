import subprocess


print('Local IP Address:\n')
print(subprocess.getoutput('hostname -I'))