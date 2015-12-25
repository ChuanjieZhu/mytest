#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void DoDegree2NMEA(double lat, double lon)
{
    int nmeaLat = 0;
    int nmeaLon = 0;

    double nmeaLat2 = 0.0;
    double nmeaLon2 = 0.0;

    double nmeaLat3 = 0.0;
    double nmeaLon3 = 0.0;
    
    nmeaLat = (int)lat;
    nmeaLon = (int)lon;
    
    nmeaLat2 = lat - nmeaLat;
    nmeaLon2 = lon - nmeaLon;
    
    nmeaLat *= 100;
    nmeaLon *= 100;

    nmeaLat2 *= 60;
    nmeaLon2 *= 60;

    nmeaLat3 = (double)nmeaLat + nmeaLat2;
    nmeaLon3 = (double)nmeaLon + nmeaLon2;

    printf("%lf, %lf\r\n", nmeaLat3, nmeaLon3);
}

int main()
{
    char line[64] = {0};
    double degreeLat = 0.0;
    double degreeLon = 0.0;

    FILE *fp = fopen("gps.txt", "at+");

    if(fp == NULL)
    {
        printf("open file error\r\n");
        return -1;
    }
    else
    {
        //fseek(fp, 0, SEEK_END);
        fseek(fp, -27, SEEK_END);
        fgets(line, 21, fp);

        printf("%s \r\n", line);
        
        sscanf(line, "%lf,%lf", &degreeLat, &degreeLon);

        printf("degreeLat %lf, degreeLon %lf \r\n", degreeLat, degreeLon);
        
        DoDegree2NMEA(degreeLat, degreeLon);

        fclose(fp);
    }

    return 0;
}
