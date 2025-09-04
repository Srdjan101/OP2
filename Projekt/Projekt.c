#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

		//Strukture
typedef struct Grana {
    int do_cvora;
    int tezina;
    struct Grana *sledeca;
} Grana;

typedef struct {
    Grana **lista_susedstva;
    int broj_cvorova;
} Graf;

typedef struct {
    int cvor;
    int udaljenost;
    int oznaka;
} CvorHeap;

typedef struct {
    CvorHeap *arr;
    int velicina, kapacitet;
} MinHeap;

		//Heap
void zameni(CvorHeap *a, CvorHeap *b) { CvorHeap t=*a; *a=*b; *b=t; }

MinHeap* napraviHeap(int kap){
    MinHeap *h = malloc(sizeof(MinHeap));
    h->arr = malloc(sizeof(CvorHeap)*kap);
    h->velicina=0; h->kapacitet=kap;
    return h;
}

void heapifyGore(MinHeap *h,int i){
    int roditelj;
    while(i>0){
        roditelj=(i-1)/2;
        if(h->arr[i].udaljenost < h->arr[roditelj].udaljenost){
            zameni(&h->arr[i],&h->arr[roditelj]);
            i=roditelj;
        } else break;
    }
}

void heapifyDole(MinHeap *h,int i){
    int l,r,sm;
    l=2*i+1;
    r=2*i+2;
    sm=i;
    if(l<h->velicina && h->arr[l].udaljenost < h->arr[sm].udaljenost) sm=l;
    if(r<h->velicina && h->arr[r].udaljenost < h->arr[sm].udaljenost) sm=r;
    if(sm!=i){zameni(&h->arr[i],&h->arr[sm]); heapifyDole(h,sm);}
}

void ubaci(MinHeap *h,int cvor,int udalj,int ozn){
    h->arr[h->velicina].cvor=cvor;
    h->arr[h->velicina].udaljenost=udalj;
    h->arr[h->velicina].oznaka=ozn;
    h->velicina++;
    heapifyGore(h,h->velicina-1);
}

CvorHeap izvuci(MinHeap *h){
    CvorHeap root=h->arr[0];
    h->arr[0]=h->arr[h->velicina-1];
    h->velicina--;
    heapifyDole(h,0);
    return root;
}

int prazno(MinHeap *h){return h->velicina==0;}

		//Graf
Graf* napraviGraf(int n){
    Graf *g=malloc(sizeof(Graf));
    int i;
    g->broj_cvorova=n;
    g->lista_susedstva=malloc(n*sizeof(Grana*));
    for(i=0;i<n;i++) g->lista_susedstva[i]=NULL;
    return g;
}

void dodajGranu(Graf *g,int u,int v,int w){
    Grana *gr=malloc(sizeof(Grana));
    gr->do_cvora=v;
    gr->tezina=w;
    gr->sledeca=g->lista_susedstva[u];
    g->lista_susedstva[u]=gr;
}

		//Generisanje grafa
Graf* generisiGraf(int n,double gustina,const char *ime_fajla){
    Graf *g=napraviGraf(n);
    int maxGrana=(int)(n*(n-1)*gustina);
    int i,u,v,w;
    FILE *f=fopen(ime_fajla,"w");
    srand(time(NULL));
    for(i=0;i<maxGrana;i++){
        u=rand()%n;
        v=rand()%n;
        if(u==v) continue;
        w=(rand()%101)-50;
        if(w==0) w=1;
        dodajGranu(g,u,v,w);
        fprintf(f,"%d %d %d\n",u,v,w);
    }
    fclose(f);
    return g;
}

		//Generesanje oznaka
void generisiOznake(int n,int *oznake,const char *ime_fajla){
    int i,idx,broj0=0,broj1=0;
    for(i=0;i<n;i++) oznake[i]=-1;
    srand(time(NULL));
    while(broj0 < 0.3*n){
        idx=rand()%n;
        if(oznake[idx]==-1){oznake[idx]=0; broj0++;}
    }
    while(broj1 < 0.4*n){
        idx=rand()%n;
        if(oznake[idx]==-1){oznake[idx]=1; broj1++;}
    }
    FILE *f=fopen(ime_fajla,"w");
    for(i=0;i<n;i++) fprintf(f,"%d %d\n",i,oznake[i]);
    fclose(f);
}

		//Oznacavanje neoznacenih
void oznaciNeoznacene(Graf *g,int *oznake,int k,int mode){
    int *pocetne_oznake=malloc(g->broj_cvorova*sizeof(int));
    int *preostalo=malloc(g->broj_cvorova*sizeof(int));
    int *broj_nula=malloc(g->broj_cvorova*sizeof(int));
    int *broj_jedan=malloc(g->broj_cvorova*sizeof(int));
    int i;

    for(i=0;i<g->broj_cvorova;i++){
        pocetne_oznake[i]=oznake[i];
        preostalo[i]=(oznake[i]==-1?k:0);
        broj_nula[i]=0;
        broj_jedan[i]=0;
    }

    MinHeap *min_skup=napraviHeap(g->broj_cvorova*2);
    for(i=0;i<g->broj_cvorova;i++){
        if(pocetne_oznake[i]==0 || pocetne_oznake[i]==1)
            ubaci(min_skup,i,0,pocetne_oznake[i]);
    }

    while(!prazno(min_skup)){
        CvorHeap ch=izvuci(min_skup);
        Grana *gr=g->lista_susedstva[ch.cvor];
        while(gr){
            int v=gr->do_cvora;
            if(oznake[v]==-1){
                if(ch.oznaka==0) broj_nula[v]++;
                else broj_jedan[v]++;
                preostalo[v]--;
                if(mode==2 && preostalo[v]<=0){
                    oznake[v]=(broj_nula[v]>=broj_jedan[v]?0:1);
                    ubaci(min_skup,v,0,oznake[v]);
                }
            }
            gr=gr->sledeca;
        }
    }

    if(mode==1){
        for(i=0;i<g->broj_cvorova;i++){
            if(oznake[i]==-1)
                oznake[i]=(broj_nula[i]>=broj_jedan[i]?0:1);
        }
    }

    free(pocetne_oznake); free(preostalo);
    free(broj_nula); free(broj_jedan);
    free(min_skup->arr); free(min_skup);
}

		//Glavni dio
int main(){
    int Ns[3]={1000,5000,10000};
    double gustine[3]={0.3,0.5,0.7};
    int Ks[8]={5,10,15,20,30,50,100,200};
    int i,j,kk,x,n,mod;
    double d;

    FILE *f_vrijeme=fopen("vremena.txt","w");
    fprintf(f_vrijeme,"n gustina k mod vrijeme\n");

    for(i=0;i<3;i++){
        for(j=0;j<3;j++){
            n=Ns[i];
            d=gustine[j];

            char ime_graf[64], ime_oznaka[64];
            sprintf(ime_graf,"graf_%d_%.1f.txt",n,d);
            sprintf(ime_oznaka,"oznake_%d_%.1f.txt",n,d);

            printf("Generisem graf n=%d, d=%.1f\n",n,d);
            Graf *g=generisiGraf(n,d,ime_graf);

            int *oznake=malloc(n*sizeof(int));
            generisiOznake(n,oznake,ime_oznaka);

            for(kk=0;kk<8;kk++){
                int k=Ks[kk];
                for(mod=1;mod<=2;mod++){
                    int *privremene_oznake=malloc(n*sizeof(int));
                    for(x=0;x<n;x++) privremene_oznake[x]=oznake[x];

                    clock_t start=clock();
                    oznaciNeoznacene(g,privremene_oznake,k,mod);
                    clock_t end=clock();
                    double elapsed=(double)(end-start)/CLOCKS_PER_SEC;

                    fprintf(f_vrijeme,"%d %.1f %d %d %.6f\n",n,d,k,mod,elapsed);
                    printf("n=%d d=%.1f k=%d mode=%d vreme=%.3f s\n",
                           n,d,k,mod,elapsed);

                    free(privremene_oznake);
                }
            }
            free(oznake);
        }
    }

    fclose(f_vrijeme);
    return 0;
}

