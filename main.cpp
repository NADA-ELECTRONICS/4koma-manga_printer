#include "mbed.h"
#include "AS289R2.h"
#include "SDFileSystem.h"

DigitalOut myled(LED1);
DigitalIn SW(P0_8);
DigitalOut sd_en(P1_16, 1);
SDFileSystem sd(P0_21, P0_22, P1_15, P0_17, "sd");
AnalogIn ain(P0_11);
AS289R2 tp(P0_19, 57600);

/**
 * FlashAir BMP(Depth=1bit) -> AS-289R2
 */
void BmpToAS289R2( int filenumber )
{
    char bmpFilename[32];
    sprintf( bmpFilename, "/sd/ttf%d.bmp", filenumber );
    FILE * fp = fopen( bmpFilename, "r" );
    if ( fp != NULL ) {
        // Get bmpOffset
        fseek( fp, 10, SEEK_SET );
        int bmpOffset = ( int )( fgetc( fp ) );
        bmpOffset += ( int )( fgetc( fp ) * 256 );
        // Get bmpHeight
        fseek( fp, 22, SEEK_SET );
        int bmpHeight = ( int )( fgetc( fp ) );
        bmpHeight += ( int )( fgetc( fp ) * 256 );
        // AS-289R2 CMD
        tp.printf( "\x1C\x2A\x65" );
        tp.putc( ( uint8_t )( bmpHeight / 256 ) );
        tp.putc( ( uint8_t )( bmpHeight % 256 ) );
        // Put Bmp-data
        for ( int iy = 1; iy <= bmpHeight; iy ++ ) {
            int LeftPoint = bmpHeight * 48 + bmpOffset - ( iy * 48 );
            fseek( fp, LeftPoint, SEEK_SET );
            for ( int ix = 0; ix < 48; ix ++ ) {
                //uint8_t imagedata = ( uint8_t )( fgetc( fp ) ^ 0xFF );
                uint8_t imagedata = ( uint8_t )( fgetc( fp ) );
                tp.putc( imagedata );
            }
        }
    }
    fclose( fp );
    free(fp);
}

int main()
{
    SW.mode( PullUp );
    tp.initialize();

    while (1) {
        if(!SW) {
            int val = ain.read_u16();
            BmpToAS289R2(val % 5);
            tp.putLineFeed(1);
            tp.printf("室外機くん (C) TOKYO FLIP-FLOP\r\r");
            tp.printf("４コマ漫画プリンタ\r");
            tp.printf("Board             Airio-Base\r");
            tp.printf("SDCard              FlashAir\r");
            tp.printf("Thermal Printer     AS-289R2\r\r");
            tp.printf("mbed+MA大祭り2018@初夏の名工大\r\r");
            tp.printf("      NADA ELECTRONICS\r");
            tp.putLineFeed(6);
        }
        myled=1;
        wait(0.1);
        myled=0;
        wait(0.1);
    }
}