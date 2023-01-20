#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <curses.h>
#include <unistd.h> 
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define MY_ADDR "10.10.10.17"
#define SERVER_ADDR "10.10.10.18"
#define SERVER_UDP_PORT 6000
#define DESTINATION_MCAST  "234.5.6.7" 
#define MAX 9

//const chtype tabChr[] = {ACS_BLOCK, ACS_BULLET, ACS_DIAMOND, ACS_PLUS, ACS_DARROW, ACS_STERLING, ACS_UARROW, ACS_VLINE, ACS_NEQUAL, ACS_HLINE};

WINDOW* big_window;
int socketfd, result, flags = 0, cli_len;
struct sockaddr_in my_address, client_address;

typedef struct {
    char addr[10];
    int init;
    int x;
    int y;
} lokStan;

lokStan glStan[9];

void clean( void )
{
    endwin();
    system( "clear" );
}

void rec()
{
    while( 1 ) 
    {
        recvfrom( socketfd, &glStan, sizeof( glStan ), flags, (struct sockaddr*)&client_address, &cli_len );

        wclear( big_window );
        box( big_window, 0, 0 );
        for( int i = 0; i < MAX; i++ ) 
        {
            if( glStan[i].init == 1 ) 
            {
            	wmove( big_window, glStan[i].y, glStan[i].x );
            	waddch( big_window, ACS_BLOCK );
            }
        }
        wrefresh( big_window );
    }
}

void init_ncurses()
{
    initscr();
    clear();
    start_color();
    init_pair( 1, COLOR_WHITE, COLOR_BLUE );
    init_pair( 2, COLOR_BLUE, COLOR_WHITE );
    init_pair( 3, COLOR_GREEN, COLOR_BLUE );
    init_pair( 4, COLOR_CYAN, COLOR_BLUE );
    cbreak();
    keypad( stdscr, TRUE );
    noecho();
    refresh();

    curs_set( 0 );
    big_window = newwin( 25, 80, 0, 0 );

    wattron( big_window, COLOR_PAIR( 1 ) );

    wbkgdset( big_window, COLOR_PAIR( 1 ) );

    wclear( big_window );
    wrefresh( big_window );

    on_exit( clean, NULL );
}

int main() 
{
    struct ip_mreqn group;
    socketfd = socket( PF_INET, SOCK_DGRAM, 0 );
    if( socketfd < 0 ) 
    {
        printf( "Socket failed, errno: %d\n", errno );
        return -1;
    }

    group.imr_address.s_addr = inet_addr(MY_ADDR);             
   group.imr_ifindex        = if_nametoindex("eth1");

   result = setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_IF, &group, sizeof(group));

   printf("setsockopt 1, result = %d\n", result);

   if (result == -1) perror("setsockopt");


  group.imr_multiaddr.s_addr = inet_addr(DESTINATION_MCAST);             
  group.imr_ifindex          = if_nametoindex("eth1");;

  result = setsockopt(socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));

  printf("setsockopt 2, result = %d\n", result);
    
    
    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = inet_addr( SERVER_ADDR ); //DESTINATION_MCAST //SERVER_ADDR
    my_address.sin_port = htons( SERVER_UDP_PORT );
    bzero( &( my_address.sin_zero ), 8 );

    
    
    result = bind( socketfd, (struct sockaddr*)&my_address, sizeof( struct sockaddr ) ); // associates a local address with a socket
    
    cli_len = sizeof( struct sockaddr );
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr( SERVER_ADDR );
    client_address.sin_port = htons( SERVER_UDP_PORT );
    bzero( &( client_address.sin_zero ), 8 );

    pthread_t      action_thr;    //watek
    pthread_attr_t action_thr_attr; //atrybuty watku

    pthread_attr_init( &action_thr_attr );
    pthread_attr_setdetachstate( &action_thr_attr, PTHREAD_CREATE_DETACHED );
    int res = pthread_create( &action_thr,
        &action_thr_attr,
        rec, NULL );

    init_ncurses();

    box( big_window, 0, 0 );

    wattron( big_window, COLOR_PAIR( 4 ) );
    wattron( big_window, A_BOLD );

    wattroff( big_window, A_BOLD );

    wattroff( big_window, COLOR_PAIR( 1 ) );
    wattron( big_window, COLOR_PAIR( 2 ) );

    wattroff( big_window, COLOR_PAIR( 2 ) );
    wattron( big_window, COLOR_PAIR( 1 ) );

    wrefresh( big_window );

    lokStan self;
    strcpy( self.addr, MY_ADDR );
    
    self.init = 2;
    result = sendto( socketfd, &self, sizeof( self ), flags, (struct sockaddr*)&client_address, cli_len );
    if( result < 0 ) printf( "BLAD" );
    
    self.init = 1;
    self.y = 1;
    self.x = 1;
    result = sendto( socketfd, &self, sizeof( self ), flags, (struct sockaddr*)&client_address, cli_len );
    if( result < 0 ) printf( "BLAD" );

    int key;
    while( ( key = getch() ) != 27 ) // ESC
    {
        switch (key)
        {
	  case KEY_UP: if (self.y > 1) self.y--; break;
	  case KEY_DOWN: if (self.y < 23) self.y++; break;
	  case KEY_LEFT: if (self.x > 1) self.x--; break;
	  case KEY_RIGHT: if (self.x < 78) self.x++; break;
        }

        result = sendto( socketfd, &self, sizeof( self ), flags, (struct sockaddr*)&client_address, cli_len );
        if( result < 0 ) printf( "BLAD" );
    }

    self.init = -1;
    result = sendto( socketfd, &self, sizeof( self ), flags, (struct sockaddr*)&client_address, cli_len );
    if( result < 0 ) printf( "BLAD" );
    
    endwin();
    system( "clear" );
}
