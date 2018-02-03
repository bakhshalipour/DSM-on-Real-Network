source info.sh
if [ "a$1" = "a" ]; then
  node="r0"
else
  node="r$1"
fi
./router.out --ip $ip --port $port --map $map --node $node --user $user --pass "$pass" --id $user
