/*
 * ow2emon.c
 * Entry point of ow2emon
 *
 * This program reads a list of one-wire devices using libowcapi (owfs)
 * and publishes the results to emoncms.
 *
 * It requires a configuration file named ow2emon.conf in the same directory
 * as the executable.
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <owcapi.h>
#include <netdb.h>
#include <netinet/in.h>

/**
 * Configuration Section
 **/

/* emoncms server name (not including http://) */
const char EmonServer[] = "emoncms.palnet.net";

/* Path to input php */
const char EmonPath[] = "emoncms";

/* Port number for emon HTTP server */
const int EmonPort = 80;

/* Node name */
const char EmonNode[] = "iridium";

/* emoncms key */
const char EmonKey[] = ;

/* List of one-wire temperature sensors to publish */
const char * EmonSensors[] = {
    "/bus.0/28.7811920A0000", /* Test1 */
    "/bus.0/28.16DD930A0000", /* Master Bedroom */
    "/bus.0/28.B467550A0000", /* Aaron's Bedroom */
    "/bus.0/28.F918930A0000", /* Kitchen */
    "/bus.0/28.738D930A0000", /* Great Room */
    "/bus.0/28.F64A930A0000", /* Study */
};

/* Names associated with each sensor above */
const char * EmonSensorNames[] = {
    "Test1",
    "MasterBedroom",
    "AaronBedroom",
    "Kitchen",
    "GreatRoom",
    "Study"
};

/* Array to store update states */
bool EmonSensorStates[sizeof(EmonSensors)/sizeof(char)];

/* Array to store sensor values */
double EmonSensorVals[sizeof(EmonSensors)/sizeof(char)];



int main(void)
{
    
    char * Buf;
    size_t S;
    
    /* Init owfs library */
    OW_init("/dev/tty.usbserial-A7009I39");
    
    /* Continue forever */
    while(1)
    {
        for(int i = 0; i < (sizeof(EmonSensors)/sizeof(char*)); i++)
        {
            /* Read owfs */
            char Name[256];
            sprintf(Name,"/uncached%s/temperature",EmonSensors[i]);
            OW_get(Name,&Buf,&S);
            
            /* Read value */
            if(Buf != 0)
            {
                EmonSensorVals[i] = atof(Buf);
                EmonSensorStates[i] = true;
            }
            else
            {
                EmonSensorVals[i] = 0.0;
                EmonSensorStates[i] = false;
            }
            
            /* Print for debugging */
            printf("Name: %s Got: %s Val: %f Good: %d\n",Name,Buf,EmonSensorVals[i],EmonSensorStates[i]);
            free(Buf);
        }
        
        /* Open socket to emoncms */
        int sockfd, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        if(sockfd < 0)
        {
            printf("ERROR: Failed to open socket\n");
            exit(-1);
        }
        
        /* Get server name */
        server = gethostbyname(EmonServer);
        
        if(server == NULL)
        {
            printf("ERROR: Failed to resolve hostname %s\n",EmonServer);
            exit(-1);
        }
        
        /* Setup socket parameters */
        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
        serv_addr.sin_port = htons(EmonPort);
        
        /* Connect to server */
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("ERROR: Failed to connect to server\n");
            exit(-1);
        }
        
        /* Write GET and initial input string to the server */
        char Temp[1024];
        sprintf(Temp,
                "GET /%s/input/post?node=%s&apikey=%s&fulljson={",
                EmonPath,
                EmonNode,
                EmonKey);
        printf("%s",Temp);
        n = write(sockfd, Temp,strlen(Temp));
        
        /* Write each sensor value */
        bool LeadingComma = false;
        for(int i = 0; i < (sizeof(EmonSensors)/sizeof(char*));i++)
        {
            if(EmonSensorStates[i])
            {
                if(LeadingComma)
                {
                    sprintf(Temp,",\"%s\":%f",EmonSensorNames[i],EmonSensorVals[i]);
                }
                else
                {
                    LeadingComma = true;
                    sprintf(Temp,"\"%s\":%f",EmonSensorNames[i],EmonSensorVals[i]);
                }
                printf("%s",Temp);
                n = write(sockfd,Temp,strlen(Temp));
            }
        }
        
        /* Write end of header */
        sprintf(Temp,"} HTTP/1.1\r\nHost: %s\r\n\r\n",EmonServer);
        printf("%s",Temp);
        n = write(sockfd,Temp,strlen(Temp));
        
        printf("Got %d\n",n);
        
        n = read(sockfd, Temp, 1025);
        printf("Got: %d\n",n);
        printf("Returned: %s\n",Temp);
        /* Close socket */
        close(sockfd);
    }
    
    /* Close */
    OW_finish();
    
    /* Open a socket to */
}
