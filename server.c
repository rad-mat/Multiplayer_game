#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_UDP_PORT 6000
#define MY_ADDR "10.10.10.18"
#define CLIENT_ADDR "10.10.10.17"
#define DESTINATION_MCAST "234.5.6.7"

#define MAX 9		// maksymalna ilość graczy
int n = 0;			// aktualna ilość graczy 

typedef struct {
    char addr[10];
    int init;		
    int x;
    int y;
} lokStan;

lokStan glStan[MAX];

void main()
{
    int sockfd, result, flags = 0, cli_len;
    struct sockaddr_in my_addr, cli_addr;

    sockfd = socket( PF_INET, SOCK_DGRAM, 0 );
    if( sockfd < 0 ) {
        printf( "socket failed\n" );
        exit( 0 );
    }
    else {
        printf( "socket successful\n" );
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons( SERVER_UDP_PORT );
    bzero( &( my_addr.sin_zero ), 8 );
    
    result = bind( sockfd, (struct sockaddr*)&my_addr, sizeof( struct sockaddr ) );
    if( result < 0 ) {
        printf( "binding failed\n" );
        exit( 0 );
    }
    else {
        printf( "binding successful\n" );
    }

    cli_len = sizeof( struct sockaddr );
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = inet_addr( DESTINATION_MCAST );
    cli_addr.sin_port = htons( SERVER_UDP_PORT );
    bzero( &( cli_addr.sin_zero ), 8 );
    
    lokStan zmiana;
    
    while(1)
    {
        result = recvfrom( sockfd, &zmiana, sizeof( zmiana ), flags, (struct sockaddr*) &cli_addr, &cli_len );
        if( result < 0 ) {
            perror( "Error with receiving zmiana!\n" );
            printf( "errno = %d\n", errno );
        }
        else {
            printf( "Receiving completed successfully, x, y = %d, %d\n", zmiana.x, zmiana.y );
        }

        if( zmiana.init == 2) {	
            for( int i = 0; i < MAX; i++ ) {
                if( glStan[i].init == 0 ) {
                    glStan[i] = zmiana;
                    break;
                }
            }
        }
        else if( zmiana.init == 1) {	
            for( int i = 0; i < MAX; i++ ) {
                if( strcmp(glStan[i].addr, zmiana.addr) == 0 ) {
                    glStan[i] = zmiana;
                    break;
                }
            }
        }
        else if( zmiana.init == -1 ) { 
            for( int i = 0; i < MAX; i++ ) {
                if( strcmp(glStan[i].addr, zmiana.addr) == 0 ) {
                    memset( glStan + i, 0, sizeof( lokStan ) );
                    break;
                }
            }
        }
        
        result = sendto( sockfd, glStan, sizeof( glStan ), flags, (struct sockaddr*)&cli_addr, cli_len );
        if( result < 0 ) {
            perror( "Error with sending updated glStan!\n" );
            printf( "errno = %d\n", errno );
        }
        else {
            printf( "Sending completed successfully\n" );
        }
        
        for( int i = 0; i < MAX; i++ ) 
        {
            printf("addr %s, init %d x %d y %d\n", glStan[i].addr, glStan[i].init, glStan[i].x, glStan[i].y);
        }
    }
}
