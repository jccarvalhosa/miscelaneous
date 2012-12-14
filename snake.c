#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <termios.h>
#define EPS 1e-9
#define TAM 30
#define MAXP 10000
#define CV 10

struct termios orig_termios;

void reset_terminal_mode() {
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode() {
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch() {
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

char tab[50][50];
double vel[2];
double pos[2];
int nx, ny, oldx, oldy;
int mvx, mvy;

struct q_t {
    int x[MAXP];
    int y[MAXP];
    int ini, fim, M;
};

typedef struct q_t queue;

void push(queue *q, int a, int b) {
    if((q->fim - q->ini + MAXP) % MAXP == q->M) q->ini = (q->ini+1) % MAXP;
    q->fim = (q->fim+1) % MAXP;
    q->x[q->fim] = a;
    q->y[q->fim] = b;
}

void init_game(queue *q) { 
    int i, j;
    q->ini=0;
    q->fim=-1; 
    q->M=4; 
    pos[0] = pos[1] = vel[0] = vel[1] = 0.0;
    nx = TAM-1-pos[1];
    ny = TAM-1-pos[0];
	mvx = mvy = 1;
    push(q, nx, ny);
    for(i=0;i<=TAM;i++) for(j=0;j<=TAM;j++) tab[i][j] = '#';
    for(i=1;i<TAM;i++) for(j=1;j<TAM;j++) tab[i][j] = ' ';
    tab[ny][nx] = 'O';
    tab[TAM-10][TAM-10] = 'X';
}

void write_game(queue *q, char c) {
    int i;
    if(q->fim < 0) return;
    if(q->ini == q->fim) tab[q->y[q->ini]][q->x[q->ini]] = c;
    else for(i=q->ini;(q->fim - i + MAXP) % MAXP != 0;i++) tab[q->y[i % MAXP]][q->x[i % MAXP]] = c;
}

void update(double ax, double ay, double t) {
    pos[1] += vel[1]*t + ax*t*t/2;
    pos[0] += vel[0]*t + ay*t*t/2;
    vel[1] += ax*t;
    vel[0] += ay*t;
    oldx = nx;
    oldy = ny;
    nx = TAM-1-pos[1];
    ny = TAM-1-pos[0];
    if(nx >= oldx+1) {
		nx=oldx+1;
		mvx=1;
		mvy=0;
	}
    if(nx <= oldx-1) {
		nx=oldx-1;
		mvx=1;
		mvy=0;
	}
    if(ny >= oldy+1) {
		ny=oldy+1;
		mvx=0;
		mvy=1;
	}
    if(ny <= oldy-1) {
		ny=oldy-1;
		mvx=0;
		mvy=1;
	}
}

void print() {
    int i, j;
    for(i=0;i<=TAM;i++) {
        for(j=0;j<=TAM;j++) printf(" %c ", tab[i][j]);
        printf("\n\r");
    }
    printf("\n\r");
}

void trata(char c) {
	if((c == 'w' || c == 'A') && mvx == 1) vel[0] = CV, vel[1] = 0;
	else if((c == 's' || c == 'B') && mvx == 1) vel[0] = -CV, vel[1] = 0;
	else if((c == 'd' || c == 'C') && mvy == 1) vel[0] = 0, vel[1] = -CV;
	else if((c == 'a' || c == 'D') && mvy == 1) vel[0] = 0, vel[1] = CV;
}

void putbite(int cont) {
    int i;
    if(tab[1 + (cont%171) % (TAM-1)][1 + cont % (TAM-1)] == ' ') tab[1 + (cont%171) % (TAM-1)][1 + cont % (TAM-1)] = 'X';
    else {
        for(i=1;i<TAM;i++) if(tab[10][i] == ' ') {
            tab[10][i] = 'X';
            break;
        }
    }
}

int findbite() {
    int i, j;
    for(i=0;i<=TAM;i++) for(j=0;j<=TAM;j++) if(tab[i][j] == 'X') return 1;
    return 0;
}

int main()
{
    int ax, ay, cont=1, i;
    clock_t t0, t1;
    char c;
    double dt;
    queue q;
    init_game(&q);
    set_conio_terminal_mode();
    t0 = clock();
	vel[0] = CV;
    while(cont++) {
        t1 = clock();
        dt = (float)(t1-t0)/CLOCKS_PER_SEC;
        t0 = t1;
        if(kbhit()) trata(getch());
        ax = 0; ay = 0;
        update(ax, ay, dt);
        if(!findbite()) putbite(cont);
        if(nx == oldx && ny == oldy) continue;
        if(tab[ny][nx] == 'X') {
            q.M++;
            putbite(cont);
        }
        if(nx <= 0 || ny <= 0 || nx >= TAM || ny >= TAM || (tab[ny][nx] == 'O' && (nx != q.x[q.ini] || ny != q.y[q.ini]))) {
			tab[ny][nx] = 'O';
			write_game(&q, ' ');
			push(&q, nx, ny);
			write_game(&q, 'O');
			system("clear");
			print();
			break;
		}
        write_game(&q, ' ');
        push(&q, nx, ny);
        write_game(&q, 'O');
        system("clear");
        print();
    }
    return 0;
}
