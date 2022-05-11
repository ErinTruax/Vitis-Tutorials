import os
import sys
import requests
import zipfile

#server_url = "https://xilinx-staging.fluidtopics.net/api/admin/khub/sources/ftml/upload"
server_url = 'https://docs.xilinx.com/api/admin/khub/sources/ftml/upload'
HEADERS = {'FT-Authorization': 'Bearer NRcvr7KGYsuYmd6KELscCnMQlA6oLmog' }
print("In uploading to FT")

  
fileobj = open ('AI.zip', 'rb')
r = requests.post(server_url, headers=HEADERS, files={"archive": ('techdocs.zip', fileobj)})
#r = requests.get(server_url +"/api/khub/maps" , headers=HEADERS)

if r.ok:
    print("OK")
else:
    print(r.status_code)
