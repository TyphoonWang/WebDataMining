#include <fstream>
#include <string>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <map>
using namespace std;
const int maxlen=100;
const int maxname=100;
const int maxsentence=3000;
#define MERGE_BIAS 1e-6 
#define MAX_ITERATION 500
#define ALPHA 0.85
char metadata[maxname]="acl-metadata.txt";
char idnetdata[maxname]="acl.txt";
typedef double real;
struct metanode
{
    char* id;
    int * author;//可能有多个author
    char* title;
    int authornum;
    int venue;
};
metanode *netnode;
int node_size=0;
int maxnodenum=100000;

char** authorindex;
int author_size=0;
int maxcoauthor=20000;

char** venueindex;
int venue_size=0;
int maxvenue_size=20000;
char readbuf[maxsentence];

bool **id_p;
bool **author_p;
bool **venue_p;

void Readmetadata()
{
    FILE *fin;
    fin=fopen(metadata,"rb");
    int flag=0;
    netnode=(metanode *)calloc(maxnodenum,sizeof(metanode));
    authorindex=(char **)calloc(maxcoauthor,sizeof(char *));
    venueindex=(char **)calloc(maxvenue_size,sizeof(char *));
    //printf("begin read metadata\n");
    while(fgets(readbuf,maxsentence,fin)!=NULL)
    {
        //printf("%s\n",readbuf);
    	int cursor=0;
    	int endcursor=0;
    	if(readbuf[cursor]=='\n')
    		continue;
    	if(readbuf[cursor]=='i'&&readbuf[cursor+1]=='d'&&flag==0)
    	{
    		while(readbuf[cursor]!='{')
    			cursor++;
    		cursor++;
    		endcursor=cursor;
    		while(readbuf[endcursor]!='}')
    			endcursor++;
    		readbuf[endcursor]=0;
            int length=endcursor-cursor;
            //if(length!=8)
            //   printf("length is %d id name : %s\n",length,readbuf+cursor);
    		netnode[node_size].id=(char *)calloc(endcursor-cursor+1,sizeof(char));
    		strcpy(netnode[node_size].id,readbuf+cursor);
            //printf("%s\n",readbuf+cursor);
    		flag=1;
    		continue;
    	}
    	else if(readbuf[cursor]=='a'&&readbuf[cursor+1]=='u'&&flag==1)
    	{
    		//处理author行字符
    		while(readbuf[cursor]!='{')
    			cursor++;
    		cursor++;
    		endcursor=cursor;
    		int curauthornum=0;
    		while(readbuf[endcursor]!=0)
    		{
    			if(readbuf[endcursor]==';'||readbuf[endcursor]=='}')
    				curauthornum++;
    			endcursor++;
    		}
    		endcursor=cursor;
            netnode[node_size].authornum=curauthornum;
    		netnode[node_size].author=(int *)calloc(curauthornum,sizeof(int));
            //printf("%d node author has been allocated\n", node_size);
    		int authorindexnum=0;
    		while(readbuf[endcursor]!=0)
    		{
    			cursor=endcursor;
                if(readbuf[cursor]==' ')
                    cursor++;
    			while(readbuf[endcursor]!=';'&&readbuf[endcursor]!='}')
    				endcursor++;
    			readbuf[endcursor]=0;
    			int length=strlen(readbuf+cursor);
                //printf("%s\n",readbuf+cursor);
    			int i;
    			for(i=0;i<author_size;++i)
    			{
    				if(strstr(authorindex[i],readbuf+cursor)!=NULL && strlen(authorindex[i])==length)
    				{
    					netnode[node_size].author[authorindexnum]=i;
    					authorindexnum++;
    					break;
    				}
    			}
    			if(i==author_size)
    			{
                   //printf("begin add new author\n");
    				netnode[node_size].author[authorindexnum]=author_size;
    				authorindexnum++;
    				authorindex[author_size]=(char *)calloc(length+1,sizeof(char));
                    strcpy(authorindex[author_size],readbuf+cursor);
                    //printf("%s\b\n",readbuf+cursor);
    				author_size++;
    			}
                if(authorindexnum==curauthornum)
                    break;
    			endcursor++;   			
       		}
       		flag=2;
            //printf("flag now is 2\n");
            continue;
    	}
    	else if(readbuf[cursor]=='t'&&readbuf[cursor+1]=='i')
    	{
    		if(flag!=2)
            {
    			printf("%s\n","format error, expected title");
                exit(0);
            }
    		while(readbuf[cursor]!='{')
    		{
    			cursor++;
    		}
    		cursor++;
    		endcursor=cursor;
    		while(readbuf[endcursor]!='}')
    		{
    			endcursor++;
    		}
    		readbuf[endcursor]=0;
    		int length=endcursor-cursor;
    		netnode[node_size].title=(char *)calloc(length+1,sizeof(char));
            //printf("%s\n",readbuf+cursor);
    		strcpy(netnode[node_size].title,readbuf+cursor);
    		flag=3;
            //printf("now flag is 3\n");
            continue;
    	}
    	else if(readbuf[cursor]=='v'&&readbuf[cursor+1]=='e')
    	{
    		if(flag!=3)
    			printf("%s\n","format error, expected venue");
    		while(readbuf[cursor]!='{')
    		{
    			cursor++;
    		}
    		cursor++;
    		endcursor=cursor;
    		while(readbuf[endcursor]!='}')
    		{
    			endcursor++;
    		}
    		readbuf[endcursor]=0;
    		int length=endcursor-cursor;
    		int i;
    		for(i=0;i<venue_size;++i)
    		{
    			if(strstr(venueindex[i],readbuf+cursor)!=NULL && strlen(venueindex[i])==length)
    			{
    				netnode[node_size].venue=i;
    				break;
    			}
    		}
    		if(i==venue_size)
    		{
    			netnode[node_size].venue=venue_size;
                venueindex[venue_size]=(char *)calloc(length+1,sizeof(char));
                strcpy(venueindex[venue_size],readbuf+cursor);
    			venue_size++;
    		}
            //printf("venue is %s",venueindex[netnode[node_size].venue]);
    		flag=4;
            //printf("now flag is 4\n");
            continue;
    	}
    	else if(readbuf[cursor]=='y'&&readbuf[cursor+1]=='e')
    	{
    		if(flag!=4)
    			printf("%s\n","format error, expected year");
    		flag=0;
    		node_size++;
            //printf("current node size :%d\n", node_size);
    		continue;
    	}
    }
    fclose(fin);
    printf("metadata acl file has been read.\n");
    printf("node_size:%d\n",node_size);
    printf("author_size:%d\n",author_size);
    printf("venue_size:%d\n",venue_size);
}

void ReadIdNet()
{
	FILE *fin;
    fin=fopen(idnetdata,"rb");
    id_p=(bool **)calloc(node_size,sizeof(bool *));
    for(int i=0;i<node_size;++i)
    {
    	id_p[i]=(bool *)calloc(node_size,sizeof(bool));
    	memset(id_p[i],0,sizeof(id_p[i]));
    }

    author_p=(bool **)calloc(author_size,sizeof(bool *));
    for(int i=0;i<author_size;++i)
    {
    	author_p[i]=(bool *)calloc(author_size,sizeof(bool));
    	memset(author_p[i],0,sizeof(author_p[i]));
    }
    venue_p=(bool **)calloc(venue_size,sizeof(bool *));
    for(int i=0;i<venue_size;++i)
    {
    	venue_p[i]=(bool *)calloc(venue_size,sizeof(bool));
    	memset(venue_p[i],0,sizeof(venue_p[i]));
    }
    int linenum=0;
    while(fgets(readbuf,maxsentence,fin)!=NULL)
    {
        linenum++;
    	int cursor=0;
    	int endcursor=0;
    	if(readbuf[0]=='\n')
    		continue;
    	while(readbuf[endcursor]!=' ')
    		endcursor++;
    	readbuf[endcursor]=0;
    	int length=endcursor-cursor;
        //printf("length:%d\n",length);
    	int i,j;
    	for(i=0;i<node_size;++i)
    	{
    		if(strstr(netnode[i].id,readbuf+cursor)!=NULL && strlen(netnode[i].id)==length)
    			break;
    	}
        if(i==node_size)
        {
            //printf("linenum: %d\n",linenum);
            //printf("%s\n",readbuf+cursor);
            //printf("format error! first ID not found \n");
            continue;
        }
        cursor=endcursor+1;
        while(readbuf[cursor]!=' ')
            cursor++;
        cursor++;
        endcursor=cursor;
        while(readbuf[endcursor]!='\n'&&readbuf[endcursor]!=0)
            endcursor++;
        readbuf[endcursor]=0;
        length=endcursor-cursor;
    	for(j=0;j<node_size;++j)
    	{
    		if(strstr(netnode[j].id,readbuf+cursor)!=NULL && strlen(netnode[j].id)==length)
    			break;
    	}
    	if(j==node_size)
    	{
            //printf("%s\n",readbuf+cursor);
    		//printf("format error! sec ID not found \n");
    		continue;
    	}
    	id_p[i][j]=true;
        //id_p[j][i]=true;
    	for(int p=0;p<netnode[i].authornum;++p)
    	{
    		for(int q=0;q<netnode[j].authornum;++q)
    		{
    			author_p[netnode[i].author[p]][netnode[j].author[q]]=true;
                //printf("author :%d %d\n",netnode[i].author[p],netnode[j].author[q]);
                //author_p[netnode[j].author[q]][netnode[i].author[p]]=true;
    		}
    	}
    	venue_p[netnode[i].venue][netnode[j].venue]=true;
        //venue_p[netnode[j].venue][netnode[i].venue]=true;
    }
    fclose(fin);
}

void PageRank(bool **network, int size, int ty)
{
    //begin id_p 
    //ty==0 ,id
    //ty==1, author
    //ty==2, venue
    printf("begin pagerank %d\n",ty);
    real **transfer;
    transfer=(real**)calloc(size,sizeof(real*));
    for(int i=0;i<size;++i)
        transfer[i]=(real*)calloc(size,sizeof(real));
    printf("the average weight of every edge of node\n");
    for(int i=0;i<size;++i)
    {
        int nearnum=0;
        for(int j=0;j<size;++j)
            if(network[i][j]==true)
                nearnum++;
        real wei;
        if(nearnum!=0)
            wei=1/real(nearnum);
        else 
            wei=1;
        for(int j=0;j<size;++j)
            if(network[i][j]==true)
                transfer[j][i]=wei;
        //printf("%d %lf\n",i,wei);
    }
    real *rankvalue=(real *)calloc(size,sizeof(real));
    real *newrankvalue=(real *)calloc(size,sizeof(real));
    real avevalue=1/real(size);
    printf("initiate the rankvalue\n");
    for(int i=0;i<size;++i)
    {
        rankvalue[i]=1;
        newrankvalue[i]=rankvalue[i];
    }//initiate the value of pagerank 
    int iteration_time=0;
    while(true)
    {
        for(int i=0;i<size;++i)
        {
            real value=0;
            for(int j=0;j<size;++j)
                value+=transfer[i][j]*rankvalue[j]*ALPHA;
            value+=(1-ALPHA)*avevalue;
            newrankvalue[i]=value;
        }
        real bias=0;
        for(int i=0;i<size;++i)
        {
            bias+=abs(newrankvalue[i]-rankvalue[i]);
        }
        bias/=size;
        printf("avebias: %lf\n",bias);
        if(bias<MERGE_BIAS)
            break;
        iteration_time++;
        if(iteration_time>MAX_ITERATION)
            break;
        for(int i=0;i<size;++i)
            rankvalue[i]=newrankvalue[i];
        printf("%d\n",iteration_time);
    }
    printf("iteration time: %d\n",iteration_time);
    FILE *fout;
    if(ty==0)//for id
    {
        fout=fopen("id_pagerank.txt","wb");
        for(int i=0;i<size;i++)
        {
            int curlen=strlen(netnode[i].id);
            for(int j=0;j<curlen;++j)
                if(netnode[i].id[j]==',')
                    netnode[i].id[j]='_';
            curlen=strlen(netnode[i].title);
            for(int j=0;j<curlen;++j)
                if(netnode[i].title[j]==',')
                    netnode[i].title[j]='_';
            int cited=0;
            for(int j=0;j<size;++j)
                if(id_p[j][i]>0)
                    cited++;
            fprintf(fout,"%s\t%lf\n",netnode[i].title, rankvalue[i],cited);
        }
    }
    else if(ty==1)
    {
        fout=fopen("author_pagerank.txt","wb");
        for(int i=0;i<size;++i)
        {
            int curlen=strlen(authorindex[i]);
            for(int j=0;j<curlen;++j)
                if(authorindex[i][j]==',')
                    authorindex[i][j]='_';
            fprintf(fout,"%s\t%lf\n",authorindex[i],rankvalue[i]);
        }
    }
    else if(ty==2)
    {
        fout=fopen("venue_pagerank.txt","wb");
        for(int i=0;i<size;++i)
        {
            int curlen=strlen(venueindex[i]);
            for(int j=0;j<curlen;++j)
                if(venueindex[i][j]==',')
                    venueindex[i][j]='_';
            fprintf(fout,"%s\t%lf\n",venueindex[i],rankvalue[i]);
        }
    }
    free(transfer);
    free(rankvalue);
    free(newrankvalue);
}
int main(int argc, char *argv[])
{
    memset(readbuf,0,sizeof(readbuf));
    Readmetadata();
    ReadIdNet();
    PageRank(id_p,node_size,0);
    PageRank(author_p,author_size,1);
    PageRank(venue_p,venue_size,2);
}
