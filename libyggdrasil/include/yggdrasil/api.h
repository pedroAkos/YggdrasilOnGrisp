#ifndef YGGDRASIL_API_H_
#define YGGDRASIL_API_H_

typedef enum __types {
  NET_MESSAGE,
  NOTIFY,
}types;

typedef struct __msg_type {
  types type;
  void* content;
}msg_type;

/* Init Yggdrasil Stack */
/* TODO add some arguments */
void init_yggdrasil(const char* ssid, const char* wifi_channel);

/* Start Yggdrasil Stack */
void start_yggdrasil();

/* Send a message to the network through Yggdrasil */
short send_msg(void* msg_contents, unsigned short msg_contents_size);

/* Receive a message from Yggdrasil */
/* This call is blocking */
/* TODO add a data structure the define what is being received */
msg_type* deliver_msg();


const char* get_ip();

#endif /* YGGDRASIL_API_H_ */
