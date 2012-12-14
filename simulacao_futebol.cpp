#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <iostream>
#include <map>
#define MAX 32

using namespace std;

const double fator_casa=0.1;
int total_gols;
int nome_max;
double media;
map<string, int> id;
int visto[32][32], gols_casa[32][32], gols_visitante[32][32];

int div_aprox(int n, int d)
{
    double doublediv = (double)n / d;
    int intdiv = n / d;
    if(doublediv - intdiv < 0.5) return intdiv;
    return intdiv + 1;
}

struct Time
{
    string nome;
    int id;
    int pontos;
    int V, E, D;
    int gp, gc, sg;
    int gols_marcados, gols_sofridos;
    double rebaixamentos, libertadores, campeonatos;

    Time() {}

    Time(int id, string nome, int atk, int def)
    {
        this->nome = nome;
        this->id = id;
        pontos = 0;
        V = E = D = 0;
        gp = gc = sg = 0;
        gols_marcados = atk;
        gols_sofridos = def;
        rebaixamentos = libertadores = campeonatos = 0;
    }

    void print()
    {
        while(nome.size() < nome_max) nome += ' ';
        printf("|");
        cout << nome;
        printf(" | %3d| %3d| %3d| %3d| %3d| %3d| %3d|%5.1lf|%5.1lf|%5.1lf|\n", 
            pontos, V, E, D, sg, gp, gc, campeonatos, libertadores, rebaixamentos);
    }
    
    void divide(int n)
    {
        pontos = div_aprox(pontos, n);
        V = div_aprox(V, n);
        E = div_aprox(E, n);
        D = div_aprox(D, n);
        gp = div_aprox(gp, n);
        gc = div_aprox(gc, n);
        sg = div_aprox(sg, n);
    }

    void zera_pontos()
    {
        pontos = 0;
        V = E = D = 0;
        gp = gc = sg = 0;
    }

    void soma(Time p)
    {
        pontos += p.pontos;
        V += p.V;
        E += p.E;
        D += p.D;
        gp += p.gp;
        gc += p.gc;
        sg += p.sg;
    }
};

Time t[MAX];
vector<Time> tabela;

struct myclass {
    bool operator() (Time a, Time b)
    {
        if(a.pontos != b.pontos) return a.pontos > b.pontos;
        if(a.V != b.V) return a.V > b.V;
        if(a.sg != b.sg) return a.sg > b.sg;
        if(a.gp != b.gp) return a.gp > b.gp;
        return a.nome < b.nome;
    }
} mycomp;

void print_tabela(int N)
{
    int i, nchar;
    nchar = nome_max + 56;
    for(i=0;i<nchar;i++) printf("-");
    printf("\n");
    printf("|Time");
    for(i=3;i<nome_max;i++) printf(" ");
    printf("|  Pt|   V|   E|   D|  Sg|  Gp|  Gc|prime|liber|rebai|\n");
    printf("|");
    for(i=0;i<nchar-2;i++) printf("#");
    printf("|\n");
    for(i=0;i<N;i++) tabela[i].print();
    for(i=0;i<nchar;i++) printf("-");
    printf("\n");
    printf("\n");
    printf("gols => %d\n", total_gols);
    printf("gols/jogo => %lf\n",(double)total_gols/(N*(N-1)));
}

int poisson(double lam)
{
    int k=0;
    double r, p=1, l = pow(sinh(1) + cosh(1), -lam);
    while(p > l)
    {
        k++;
        r = (double)rand()/(double)RAND_MAX;
        srand(rand());
        p = p*r;
    }
    return k-1;
}

pair<int, int> jogo(int a, int b)
{
    int golsa, golsb;
    int total = t[a].gols_marcados+t[b].gols_sofridos+t[b].gols_marcados+t[a].gols_sofridos;
    golsa = poisson((1.0+fator_casa)*(t[a].gols_marcados + t[b].gols_sofridos)/total*media);
    golsb = poisson((1.0-fator_casa)*(t[b].gols_marcados + t[a].gols_sofridos)/total*media);
    return make_pair(golsa, golsb);
}

void simula_jogo(int a, int b)
{
    pair<int, int> placar;
    int golsa, golsb;
    if(!visto[a][b]) placar = jogo(a, b);
    else placar = make_pair(gols_casa[a][b], gols_visitante[a][b]);
    golsa = placar.first;
    golsb = placar.second;
    total_gols += golsa + golsb;

    if(golsa > golsb)
    {
        t[a].pontos += 3;
        t[a].V++;
        t[b].D++;
    }
    else if(golsa == golsb)
    {
        t[a].pontos++;
        t[b].pontos++;
        t[a].E++;
        t[b].E++;
    }
    else
    {
        t[b].pontos += 3;
        t[b].V++;
        t[a].D++;
    }
    t[a].sg += golsa - golsb;
    t[a].gc += golsb;
    t[a].gp += golsa;
    t[b].sg += golsb - golsa;
    t[b].gp += golsb;
    t[b].gc += golsa;
}

void simula_campeonatos(int N, int k)
{
    Time tbackup[32];
    int i, j, cont;
    for(i=0;i<N;i++) tbackup[i] = t[i];
    for(cont=0;cont<k;cont++)
    {
        for(i=0;i<N;i++) t[i].zera_pontos();
        for(i=0;i<N;i++) for(j=0;j<N;j++) if(j != i) simula_jogo(i, j);
        for(i=0;i<N;i++) tabela.push_back(t[i]);
        sort(tabela.begin(), tabela.end(), mycomp);
        tbackup[tabela[0].id].campeonatos++;
        for(i=0;i<4;i++) tbackup[tabela[i].id].libertadores++;
        for(i=N-4;i<N;i++) tbackup[tabela[i].id].rebaixamentos++;
        for(i=0;i<N;i++) tbackup[i].soma(t[i]);
        tabela.clear();
    }
    for(i=0;i<N;i++) t[i] = tbackup[i];
    for(i=0;i<N;i++)
    {
        t[i].divide(k);
        t[i].libertadores = t[i].libertadores / k * 100;;
        t[i].rebaixamentos = t[i].rebaixamentos / k * 100;
        t[i].campeonatos = t[i].campeonatos / k * 100;
        tabela.push_back(t[i]);
    }
    total_gols /= k;
}

int indice(string nome)
{
    if(!id.count(nome))
    {
        id[nome] = id.size() - 1;
        nome_max = max(nome_max,(int)nome.size());
        t[id.size()-1] = Time(id.size()-1, nome, 0, 0);
        return id.size() - 1;
    }
    return id[nome];
}

void read(int N, int rodada)
{
    int i, j, a, b, g1, g2;
    string nome1, nome2;
    total_gols = 0;
    tabela.clear();
    nome_max = 0;
    for(i=0;i<N;i++) for(j=0;j<N;j++) visto[i][j] = 0;
    for(i=0;i<rodada;i++)
    {
        for(j=0;j<N/2;j++)
        {
            cin >> nome1 >> g1 >> nome2 >> g2;
            a = indice(nome1);
            b = indice(nome2);
            visto[a][b] = 1;
            t[a].gols_marcados += g1;
            t[a].gols_sofridos += g2;
            t[b].gols_marcados += g2;
            t[b].gols_sofridos += g1;
            gols_casa[a][b] = g1;
            gols_visitante[a][b] = g2;
            media += g1 + g2;
        }
    }
    media /= (rodada * N/2);
}

int main()
{
    srand( time(NULL) );
    int N, i, j, k, rodada;
    while(scanf("%d %d", &N, &rodada) && N)
    {
        read(N, rodada);
        simula_campeonatos(N, 10000);
        sort(tabela.begin(), tabela.end(), mycomp);
        print_tabela(N);
    }
    return 0;
}

