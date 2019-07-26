% @doc myapp public API.
% @end
-module(myapp).

-export ([init_nif/0, foo/1, bar/1, loop/0, init_ygg_nif/0, print_ip/0, init_ygg_messaging/0]).

-on_load(init_ygg_nif/0).

-behavior(application).

% Callbacks
-export([start/2]).
-export([stop/1]).

%--- Callbacks -----------------------------------------------------------------

start(_Type, _Args) ->
  grisp_led:color(1, blue),
  myapp_sup:start_link().

stop(_State) -> ok.

add_host(IP, NAME) ->
  io:format("Adding host ~s@~s ~n", [IP, NAME]),
  case inet:parse_address(IP) of
    {ok, {XX,YY,ZZ,WW}} ->
      inet_db:add_host({XX,YY,ZZ,WW}, [NAME]);
    {error, einval} ->
      logger:error("Error parsing ip: ~s ~n ", [IP])
  end.

loop() ->
  receive
    {msg, String} ->
      io:format("~s~n", [String]);
    {notify, String} ->
      io:format("~s~n", [String]);
    {notify, IP, NAME} ->
      add_host(IP, NAME);
    true ->
      io:format("Received something~n")
    end,
  loop().

get_soname() ->
  filename:join(case code:priv_dir(?MODULE) of
        {error, bad_name} ->
          %% this is here for testing purposes
          filename:join(
          [filename:dirname(code:which(?MODULE)),"..","priv"]);
        Dir ->
          Dir
        end,
        "myapp").


init_ygg_nif() ->
  SoName = get_soname(),
  ok = erlang:load_nif(SoName, 0),
  init_ygg().

init_ygg_messaging() ->
  Pid=spawn(?MODULE, loop, []),
  init_ygg_messaging(Pid).

init_ygg() ->
  io:format("Yggdrasil not Loaded~n").

init_ygg_messaging(Pid) ->
  io:format("Yggdrasil not Loaded ~n").

print_ip() ->
  io:format("My ip is ~s ~n", [get_ip()]).

get_ip() ->
  "Not known".

init_nif() ->
  SoName = get_soname(),

    Pid=spawn(?MODULE, loop, []),
    foobar(Pid),
    ok = erlang:load_nif(SoName, 0),
    foobar(Pid),
    ok.


foobar(Pid) ->
  io:format("~w~n", [Pid]).

foo(_X) ->
    io:format("foo~n").
bar(_Y) ->
    io:format("bar~n").
