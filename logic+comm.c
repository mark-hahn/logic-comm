#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libserialport.h>

void printErr(const char* hdr1, const char* hdr2, sp_return ret) {
  if (ret < 0) {
    char errStr[15];
    switch (ret) {
      case SP_ERR_ARG:  strcpy(errStr, ", SP_ERR_ARG");  break;
      case SP_ERR_FAIL: strcpy(errStr, ", SP_ERR_FAIL"); break;
      case SP_ERR_MEM:  strcpy(errStr, ", SP_ERR_MEM");  break;
      case SP_ERR_SUPP: strcpy(errStr, ", SP_ERR_SUPP"); break;
      default: strcpy(errStr, ""); break;
    }
    if(ret == SP_ERR_FAIL) {
      char* msg = sp_last_error_message();
      printf("%s%s %s, %s\n", hdr1, hdr2, errStr, msg);
      sp_free_error_message(msg);
    } else printf("%s%s%s\n", hdr1, hdr2, errStr);
  }
}

int main(int argc, char** argv)  {
  sp_return ret;
  char *portName = NULL;
  bool listPorts = false;
  int index;
  int c;

  opterr = 0;
  while ((c = getopt (argc, argv, "lp:")) != -1) {
    switch (c) {
      case 'p':
        portName = optarg;
        break;
      case 'l':
        listPorts = true;
        break;
      case '?':
        if (optopt == 'p')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        return 1;
      default:
        abort ();
    }
  }
  for (index = optind; index < argc; index++)
    printf ("Unknown argument ignored: %s\n", argv[index]);

  if (!portName && !listPorts) {
    // printf ("Nothing to do\n");
    portName = (char*) "/dev/ttyACM0";
    // return 0;
  }

  int i;
  struct sp_port **ports;
  struct sp_port *port = NULL;

  sp_list_ports(&ports);
  for (i=0; ports[i]; i++) {
    if (listPorts)
      printf ("Found port %s\n", sp_get_port_name(ports[i]));
    if (portName && !strcmp(sp_get_port_name(ports[i]), portName)) port = ports[i];
  }
  if (!portName) return 0;

  if (!port) {
    printErr("Unable to find port ", portName, (sp_return) 1);
    return 1;
  }
  ret = sp_open(port, SP_MODE_READ_WRITE);
  printErr("Unable to open port ", sp_get_port_name(port), ret);

  // struct sp_port_config* config;
  // int baudrate;
  // ret = sp_new_config(&config);
  // printErr("sp_new_config err", "", ret);
  // ret = sp_get_config(port, config);
  // printErr("sp_get_config err", "", ret);
  // ret = sp_get_config_baudrate(config, &baudrate);
  // printErr("sp_get_config_baudrate err", "", ret);
  // printf("baudrate %d\n", baudrate);

  ret = sp_set_baudrate(port, 115200);
  printErr("sp_set_baudrate err, ", "115200", ret);
  ret = sp_set_bits(port, 8);
  printErr("sp_set_bits err, ", "8", ret);
  ret = sp_set_parity(port, SP_PARITY_NONE);
  printErr("sp_set_parity err, ", "SP_PARITY_NONE", ret);
  ret = sp_set_stopbits(port, 1);
  printErr("sp_set_stopbits err, ", "1", ret);
  ret = sp_flush(port, SP_BUF_BOTH);
  printErr("sp_flush err, ", "1", ret);

  printf("Listening on %s\n", portName);

  char wbuf[101] = "";
  char rbuf[101] = "";
  while(1) {
    int bytesRead = sp_nonblocking_read(port, rbuf, 100);
    printErr("sp_nonblocking_write err, ", "", (sp_return) bytesRead);
    rbuf[bytesRead] = 0;
    printf("%s", rbuf);

    // sleep(1);
    wbuf[0] = '0';
    ret = sp_blocking_write(port,wbuf,1,0);
    printErr("sp_nonblocking_write err, ", "", ret);
  }
}
