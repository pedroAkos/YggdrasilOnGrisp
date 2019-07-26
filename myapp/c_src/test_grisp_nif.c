
#include <erl_nif.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

static void* foo2() {
  while(1) {
    sleep(1);
    printf("hello\n");
  }
}

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

  ret = bar(y);
  return enif_make_int(env, ret);
}


static ErlNifFunc nif_funcs[] = {
    {"foo", 1, foo_nif},
    {"bar", 1, bar_nif},
    {"foobar", 1, foobar_nif}
};

ERL_NIF_INIT(myapp, nif_funcs, NULL, NULL, NULL, NULL)
