#include "enum.h"
#include "doexec.h"

char *linux_enum(){
    /*Headers printed during linux enumeration*/
    char *linEnum = "\nLinux Enumeration Starting\n";
    char *host_head = "\nHostname Info\n";
    char *kernal_head = "\nKerbal Info\n";
    char *root_serv_head = "\nRoot Services\n";
    char *net_head = "\nNetwork Information\n";

    /*System commands for linux enumeration*/
    char *host_info = doexec("hostname");
    char *kernal_info = doexec("uname -a");
    char *root_serv_info = doexec("ps aux | grep root");
    char *ip_info = doexec("ifconfig");
    char *ports_info = doexec("lsof -i");

    /*Constructing linux enum buffer*/
    int size = (strlen(linEnum) 
                + strlen(host_head)
                + strlen(kernal_head)
                + strlen(root_serv_head)
                + strlen(host_head)
                + strlen(net_head)
                + strlen(host_info)
                + strlen(kernal_info)
                + strlen(root_serv_info)
                + strlen(ip_info)
                + strlen(ports_info)
                ) * sizeof(char);
   
    char *result = malloc(size);
    
    /*Storing linux enum result*/
    strcat(result, linEnum);
    strcat(result, host_head);
    strcat(result, host_info);
    strcat(result, kernal_head);
    strcat(result, kernal_info);
    strcat(result, root_serv_head);
    strcat(result, root_serv_info);
    strcat(result, net_head);
    strcat(result, ip_info);
    strcat(result, ports_info);

    /*Freeing doexec char pointers*/
    free(host_info);
    free(kernal_info);
    free(root_serv_info);
    free(ip_info);
    free(ports_info);
    return result;

}