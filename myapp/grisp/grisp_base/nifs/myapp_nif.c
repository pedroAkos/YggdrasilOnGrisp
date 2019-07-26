#define STATIC_ERLANG_NIF 1

#include <erl_nif.h>
#include <yggdrasil/api.h>
#include <pthread.h>
#include <time.h>

int inited = 0;

int foo(int x) {
  return x+1;
}

int bar(int y) {
  return y*2;
}

static ERL_NIF_TERM foo_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{

    int x, ret;
    if (!enif_get_int(env, argv[0], &x)) {
	return enif_make_badarg(env);
    }
    ret = foo(x);
    return enif_make_int(env, ret);
}

static ERL_NIF_TERM bar_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    int y, ret;
    if (!enif_get_int(env, argv[0], &y)) {
	return enif_make_badarg(env);
    }
    ret = bar(y);
    return enif_make_int(env, ret);
}

typedef struct __erlang_shit {
  ErlNifPid pid;
  ERL_NIF_TERM term;
}erlang_shit;

static void* send_for_eternity(erlang_shit* s) {
  while(1) {
    sleep(1);
    if(enif_send(NULL, &s->pid, NULL, s->term))
    printf("  send succeed\n");
    else
    printf(" send failed\n");
  }
  return NULL;
}

static ERL_NIF_TERM foobar_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int y, ret;
  erlang_shit* s = malloc(sizeof(erlang_shit));
  //ErlNifPid pid;
  if (!enif_get_local_pid(env, argv[0], &s->pid))
    return enif_make_badarg(env);

  //ErlNifEnv* msg_env = enif_alloc_env();
  //ERL_NIF_TERM msg = enif_make_atom(msg_env, "msg");
  ERL_NIF_TERM msg = enif_make_atom(env, "msg");

  //const char* hello_str = "hello";
  //ERL_NIF_TERM hello = enif_make_string(msg_env, "hello_str", ERL_NIF_LATIN1);
  ERL_NIF_TERM hello = enif_make_string(env, "hello_str", ERL_NIF_LATIN1);

  //ERL_NIF_TERM tu1 = enif_make_tuple1(msg_env, hello);
  //ERL_NIF_TERM tu2 = enif_make_tuple2(msg_env, msg, tu1);

  ERL_NIF_TERM tu1 = enif_make_tuple1(env, hello);
  s->term = enif_make_tuple2(env, msg, tu1);

  pthread_t t;
  pthread_create(&t, NULL, send_for_eternity, s);

  //ret = bar(y);
  return enif_make_atom(env, "ok");
}

typedef struct _wrapper {
  ErlNifPid pid;
}erlang_pid;

static void handle_comm(erlang_pid* p) {
  while(1) {
  msg_type* msg = deliver_msg();
  if(msg->type == NOTIFY) {
    ErlNifEnv* env = enif_alloc_env();
    ERL_NIF_TERM noti = enif_make_atom(env, "notify");
    ERL_NIF_TERM value = enif_make_string(env, msg->content, ERL_NIF_LATIN1);
    ERL_NIF_TERM tuple = enif_make_tuple2(env, noti, value);
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
    {"foo", 1, foo_nif},
    {"bar", 1, bar_nif},
    {"init_ygg", 0, init_ygg_nif},
    {"init_ygg_messaging", 1, init_ygg_messaging_nif},
    {"get_ip", 0, get_ip_nif}

};

ERL_NIF_INIT(myapp, nif_funcs, NULL, NULL, NULL, NULL)
