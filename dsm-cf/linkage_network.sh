source info.sh

rm noc-log/*

for i in {0..15}
do
	./router.out --ip $ip --port $port --map $map --node "r$i" --user $user --pass "$pass" --id $user > noc-log/"router$i".txt &
	sleep 1
done

tail -f noc-log/*

