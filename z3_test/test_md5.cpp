#include "md5.h"
  
int main( int argc, char *argv[] )
{
    int j,k;
    const char *msg = "The quick brown fox jumps over the lazy dog.";
    char buffer[250];
    unsigned int val = 0x7ac979ac;
    //unsigned int val = 0x4f4d4e4c;
/*    unsigned char *p = (unsigned char *)&val;
    sprintf(buffer, msg, p[0], p[1], p[2], p[3]);
      printf("Hello\n");*/
    unsigned *d = ::md5(msg, strlen(msg));
    WBunion u;
 
    for (j=0;j<4; j++){
        u.w = d[j];
        for (k=0;k<4;k++) printf("%02x",u.b[k]);
    }
    printf("\n");
 
    return 0;
}