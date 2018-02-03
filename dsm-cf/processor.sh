source info.sh
if [ "a$1" = "a" ]; then
  node="p0"
else
  node="p$1"
fi
./processor.out --ip $ip --port $port --map $map --node $node --user $user --pass "$pass" --id $user
