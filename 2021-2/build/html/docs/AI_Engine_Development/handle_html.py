import sys,os
dirname = os.path.dirname(__file__)
print(dirname)
from bs4 import BeautifulSoup

def handle_html(filename):
    file = open(filename, 'r+', encoding="utf-8")
    soup = BeautifulSoup(file, 'html.parser')
    soup = soup.find('div', {"class": "rst-content"})
    if soup == None:
            return
    sections = soup.findAll('div', {"class": "section"})
    documents = soup.findAll('div', {"class": "document"})
    navigation_bar = soup.find('div', {"role": "navigation"})
    if navigation_bar:
        navigation_bar.decompose()
    print(len(documents))
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
    soup2 = BeautifulSoup(str(soup), 'html.parser')
    soup2.contents[0].unwrap()


    file.write(str(soup2))


directory = '.'
for root, dirnames, filenames in os.walk(directory):
    for filename in filenames:
        if filename.endswith('.html'):
            fname = os.path.join(root, filename)
            handle_html(fname)
            
