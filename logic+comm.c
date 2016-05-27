#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libserialport.h>

void printErr(const char* hdr1, const char* hdr2, sp_return ret) {
  if (ret) {
    char errStr[15];
    switch (ret) {
      case SP_OK:       strcpy(errStr, ", SP_OK");       break;
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

  int i;
  struct sp_port **ports;
  struct sp_port *port = NULL;

  sp_list_ports(&ports);
  for (i=0; ports[i]; i++) {
    if (listPorts)
      printf ("Found port %s\n", sp_get_port_name(ports[i]));
    if (portName && !strcmp(sp_get_port_name(ports[i]), portName)) port = ports[i];
  }
  if (!portName) return 1;
  if (!port) {
    printErr("Unable to find port ", portName, (sp_return) 1);
    return 1;
  }
  ret = sp_set_baudrate(port, 115200);
  printErr("sp_set_baudrate err, ", "115200", ret);
  ret = sp_set_bits(port, 8);
  printErr("sp_set_bits err, ", "8", ret);
  ret = sp_set_parity(port, SP_PARITY_NONE);
  printErr("sp_set_parity err, ", "SP_PARITY_NONE", ret);
  ret = sp_set_stopbits(port, 1);
  printErr("sp_set_stopbits err, ", "1", ret);
  ret = sp_open(port, SP_MODE_READ);
  printErr("Unable to open port ", sp_get_port_name(port), ret);

  printf("sp_input_waiting: %d\n", sp_input_waiting(port));

  char chars[101] = "";
  int bytesRead = sp_blocking_read_next(port, chars, 100, 0);
  if (bytesRead < 0) {
    printf("Invalid data read from port: %d\n", bytesRead);
    return 1;
  }
  chars[bytesRead] = 0;
  printf("%s\n", chars);

  sp_close(port);
  sp_free_port_list(ports);
}
