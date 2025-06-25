#!/bin/sh


PATH="/sbin:/bin:/usr/sbin:/usr/bin"
NAME="aesdsocket-start-stop"
DESC="aesdsocket application"

start() {
    start-stop-daemon -S -n aesdsocket --exec /usr/bin/aesdsocket -- -d

}
   
stop() {
    start-stop-daemon --stop --signal SIGTERM --name aesdsocket --retry 3 

}

restart(){
    stop
    sleep 10
    start
}
case "${1}" in
        start)
            start
                ;;

        stop)
            stop
         
                ;;

        restart|force-reload)
            restart
                ;;

        *)
                echo "Usage: ${0} {start|stop|restart|force-reload}" >&2
                exit 1
                ;;
esac

exit 0