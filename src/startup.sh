declare -a command_list
declare -i command_count
source `pwd`/tsr_config

if is_valid_address $login_server_addr; then
    ip = ${$login_server_addr##*:}
    echo $ip
	add_command "$CMD"
fi

function add_command()
{
	command_list[$command_count]="$1"
	command_count=$command_count+1
}

function start()
{
	for c in "${command_list[@]}"; do
		echo "$c" | awk '{print $1}' | xargs basename | xargs -I{} printf "Starting % -50s" "{} ..."
		$c
		sleep 1
		if $(ps2 "$c"); then
			echo -ne "[FAILED]\n"
		else
			echo -ne "[  OK  ]\n"
		fi
	done
}

function stop()
{
	for c in "${command_list[@]}"; do
		echo "$c" | awk '{print $1}' | xargs basename | xargs -I{} printf "Stopping % -50s" "{} ..."
		pkill -f "$c"
		sleep 1
		if $(ps2 "$c"); then
			echo -ne "[  OK  ]\n"
		else
			echo -ne "[FAILED]\n"
		fi
	done
}
# case "$1" in
# 	start)
# 		start
# 		;;
# 	stop)
# 		countdown "关闭！" $2
# 		stop
# 		;;
# 	restart)
# 		countdown "重新启动！" $2
# 		stop
# 		sleep 3
# 		start
# 		;;
# 	status)
# 		pgrep -u $(id -u) "loginserver|proxyserver|channelserver|gameserver|socialserver|matchingserver|transitserver|authenticationserver" | xargs -r ps -o pid,comm,time,wchan
# 		;;
# 	kill)
# 		pgrep -u $(id -u) "loginserver|proxyserver|channelserver|gameserver|socialserver|matchingserver|transitserver|authenticationserver" | xargs -r kill
# 		;;
# 	*)
# 		echo "Usage: $0 (start|stop|restart|status)"
# 		exit 1
# 		;;
# esac

# cd bin
# ./channelserver --ip=192.168.1.168 --data_path=./../../exe/Config.json & 
# ./loginserver --data_path=./../../exe/loginserver.json &
