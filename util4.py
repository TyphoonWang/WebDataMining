import os, io
import re
import numpy as np
import bs4
from subprocess import call
from bs4 import BeautifulSoup as bsoup
from xml.dom.minidom import parse
import codecs
from textrank4zh import TextRank4Keyword, TextRank4Sentence
from collections import Counter
import sys

def extract_keyword(text):
    tr4w=TextRank4Keyword()
    tr4w.analyze(text=text, lower=True, window=2)
    item_list=tr4w.get_keywords(3, word_min_len=1)
    phrase_list=tr4w.get_keyphrases(keywords_num=3, min_occur_num= 2)
    return item_list, phrase_list
def extract_abstract(text):
    tr4s=TextRank4Sentence()
    tr4s.analyze(text=text, lower=True,source='all_filters')
    sentence_list=tr4s.get_key_sentences(num=3)
    return sentence_list
def load_wiki(question_list, file_name):
    outfile=io.open("processedwiki_%s" % (file_name.split(".")[0]),"wb")
    print>>outfile, "<wikipedia>"    
    phraseset=set()
    for q in question_list:
        q=q.split("::")[0]
        [qitem, qphrase]=extract_keyword(q)
        for item in qitem:
            phraseset.add(item.word)
    with io.open(file_name,'r') as infile:
        page_list=bsoup(infile,"html.parser").find_all('page')
        print "pagesize:"+str(len(page_list))
        titledel = re.compile(u"\u5220\u9664\u7eaa\u5f55|Upload log")
        textref=re.compile(u"<ref[\S\s]+/ref>|<reference[\S\s]+/reference>")
        for page in page_list:
            title=page.title
            if re.search(titledel,title.text):
                continue
            text=page.revision.text
            flag=0
            #print page.title
            [itemlist, phraselist]=extract_keyword(page.title.text)
            #print itemlist[0].word
            if len(itemlist)<1:
                continue
            if itemlist[0].word in phraseset:
                flag=1
            if flag==0:
                continue
            print title.text.encode('utf-8')
            rid=page.revision.id 
            print>>outfile,"<page>"
            print>>outfile, title.encode('utf-8')
            print>>outfile, rid.encode('utf-8')
            #sentencelist=extract_abstract(text)
            #print>>outfile,"<abstract>"
            #for item in sentencelist:
            #    print>>outfile,item.sentence
            #print>>outfile,"</abstract>"
            print>>outfile, "<corpus>"
            print>>outfile,text
            print>>outfile,"</corpus>"
            print>>outfile,"</page>"
    print>>outfile,"</wikipedia>"
    outfile.close()

def load_question(file_name):
    qlist=[]
    with io.open(file_name,'r') as infile:
        qlines=infile.readlines()
        for line in qlines:
            line=line.strip()
            columns=re.split(u'[\s|\uff1f|?]+',line)
            print columns[0]
            qlist.append(columns[0]+"::"+columns[1])
    return qlist
def matchqa(qlist, wikifile, outfile):
    fileout=io.open(outfile,"wb")
    with io.open(wikifile,'r') as infile:
        page_list=bsoup(infile,"html.parser").find_all('page')
        qindex=1
        for question in qlist:
            realq=question.split('::')[0]
            reala=question.split('::')[1]
            [qitem, qphrase]=extract_keyword(realq)
            keywords=set()
            for item in qitem:
                keywords.add(item.word)
            for item in qphrase:
                keywords.add(item)
            print>>fileout, "?"+str(qindex)+"."+realq+":"+reala
            for page in page_list:
                for item in keywords:
                    if page.title.text.find(item)!=-1:
                        print>>fileout,"Title: "+page.title.text.encode('utf-8')
                        print>>fileout,"Snippet: "+page.corpus.text.encode('utf-8')
            qindex=qindex+1
    fileout.close()
if __name__=="__main__":
    #filename="excerpt.txt"
    filename="zhwiki-20161101-pages-articles-multistream_1.xml"
    question_list=load_question("question.txt")
    load_wiki(question_list, filename)
    matchqa(question_list,"processedwiki_%s" % (filename.split(".")[0]), "material_%s.txt"% (file_name.split('.')[0]) )
