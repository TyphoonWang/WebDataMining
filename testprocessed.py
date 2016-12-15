import os, io
import re
import numpy as np
import bs4
from subprocess import call
from bs4 import BeautifulSoup as bsoup
from xml.dom.minidom import parse
from collections import Counter

def load_processed(file_name):
    with io.open(file_name,'r') as infile:
        page_list=bsoup(infile,"html.parser").find_all('page')
        for page in page_list:
            title=page.title
            print title.encode('utf-8')

if __name__=="__main__":
    filename="processedwiki_1.xml"
    #filename="zhwiki-20161101-pages-articles-multistream_1.xml"
    load_processed(filename)
