import os
import sys
import requests
import zipfile

#server_url = "https://xilinx-staging.fluidtopics.net/api/admin/khub/sources/ftml/upload"
server_url = 'https://docs.xilinx.com/api/admin/khub/sources/ftml/upload'
HEADERS = {'FT-Authorization': 'Bearer NRcvr7KGYsuYmd6KELscCnMQlA6oLmog' }
print("In uploading to FT")
zf = zipfile.ZipFile('techdocs.zip', "w")
for dirname, subdirs, files in os.walk("."):
    print("dirname " + dirname)
    zf.write(dirname)
    for filename in files:
        print("filename " +filename)
        zf.write(os.path.join(dirname, filename))
zf.close()
  
fileobj = open ('techdocs.zip', 'rb')
r = requests.post(server_url, headers=HEADERS, files={"archive": ('techdocs.zip', fileobj)})
#r = requests.get(server_url +"/api/khub/maps" , headers=HEADERS)

if r.ok:
    print("OK")
else:
    print(r.status_code)
