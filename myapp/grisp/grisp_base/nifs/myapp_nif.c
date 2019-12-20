#define STATIC_ERLANG_NIF 1

#include <erl_nif.h>
#include <yggdrasil/api.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

int inited = 0;


typedef struct _wrapper {
  ErlNifPid pid;
}erlang_pid;

static void handle_comm(erlang_pid* p) {
  while(1) {
  msg_type* msg = deliver_msg();
  if(msg->type == NOTIFY) {
    ErlNifEnv* env = enif_alloc_env();
    ERL_NIF_TERM noti = enif_make_atom(env, "notify");
    char ip[16];
    void* ptr = msg->content;
    memcpy(ip, ptr, 16); ptr+=16;
    int hostname_len;
    memcpy(&hostname_len, ptr, sizeof(int)); ptr+=sizeof(int);
    char* hostname[hostname_len];
    memcpy(hostname, ptr, hostname_len);

    ERL_NIF_TERM ip_term = enif_make_string(env, ip, ERL_NIF_LATIN1);
    ERL_NIF_TERM hostname_term = enif_make_string(env, hostname, ERL_NIF_LATIN1);

    ERL_NIF_TERM tuple = enif_make_tuple3(env, noti, ip_term, hostname_term);
    enif_send(NULL, &p->pid, env, tuple);
    enif_free_env(env);
  } else if(msg->type = NET_MESSAGE) {
    ErlNifEnv* env = enif_alloc_env();
    ERL_NIF_TERM noti = enif_make_atom(env, "msg");
    ERL_NIF_TERM value = enif_make_string(env, msg->content, ERL_NIF_LATIN1);
    ERL_NIF_TERM tuple = enif_make_tuple2(env, noti, value);
    enif_send(NULL, &p->pid, env, tuple);
    enif_free_env(env);
  }

  free(msg->content);
  free(msg);
}
}

static ERL_NIF_TERM init_ygg_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {

  start_yggdrasil();

  return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM init_ygg_messaging_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {


  erlang_pid* p = malloc(sizeof(erlang_pid));
  if (!enif_get_local_pid(env, argv[0], &p->pid))
    return enif_make_badarg(env);

  pthread_t handle_comm_thread;
  pthread_create(&handle_comm_thread, NULL, handle_comm, p);

  return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM get_ip_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  char* ip = get_ip();
  return enif_make_string(env, ip, ERL_NIF_LATIN1);
}

static ErlNifFunc nif_funcs[] = {
    {"init_ygg", 0, init_ygg_nif},
    {"init_ygg_messaging", 1, init_ygg_messaging_nif},
    {"get_ip", 0, get_ip_nif}

};

ERL_NIF_INIT(myapp, nif_funcs, NULL, NULL, NULL, NULL)
