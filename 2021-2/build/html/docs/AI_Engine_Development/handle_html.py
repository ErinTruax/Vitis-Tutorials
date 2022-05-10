import sys,os
import requests
import zipfile
dirname = os.path.dirname(__file__)
print(dirname)
from bs4 import BeautifulSoup

def handle_html(filename):
    print(filename)
    file = open(filename, 'r+', encoding="utf-8")
    soup = BeautifulSoup(file, 'html.parser')
    sections = soup.findAll('div', {"class": "section"})
    documents = soup.findAll('div', {"class": "document"})
    articleBodys = soup.findAll('div', {"itemprop": "articleBody"})

    headerlinks = soup.findAll('a', {'class': 'headerlink'})
    toctree_wrappers = soup.findAll('div', {'id': 'table-of-contents'})
    
    first_h1 = soup.find('h1')
    if first_h1 != None:
        first_h1.decompose()

    for section in sections:
        section.unwrap()
    for document in documents:
        print('unwrap document')
        document.unwrap()
    for articleBody in articleBodys:
        articleBody.unwrap()
    for headerlink in headerlinks:
        headerlink.decompose()

    for toctree in toctree_wrappers:
        toctree.decompose()    

    file.seek(0)
    file.truncate()


    file.write(str(soup))


directory = '.'
for root, dirnames, filenames in os.walk(directory):
    for filename in filenames:
        if filename.endswith('.html'):
            fname = os.path.join(root, filename)
            handle_html(fname)

print("HERE")



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
  
#fileobj = open ('techdocs.zip', 'rb')
#r = requests.post(server_url, headers=HEADERS, files={"archive": ('techdocs.zip', fileobj)})
#r = requests.get(server_url +"/api/khub/maps" , headers=HEADERS)

#if r.ok:
#    print("OK")
#else:
#    print(r.status_code)
            
