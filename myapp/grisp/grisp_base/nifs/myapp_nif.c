#define STATIC_ERLANG_NIF 1

#include <erl_nif.h>
#include <yggdrasil/api.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

int inited = 0;


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
