#!/bin/bash


function enter_qrcode()
{
    ZBAR_STDOUT=$(mktemp)
    ZBAR_STDERR=$(mktemp)

    coproc ZBAR { zbarcam --raw --nodisplay ; }

    ZBAR_STDOUT=

    zbar_pid=$ZBAR_PID

    while :
    do
        # TASK 1
        read -t 0.2 -n 1 key

        if [[ $key = q ]]
        then
            #echo exit due key event
            reason=key
            break
        fi
        
        read -t 0.2 QRCODE <&${ZBAR[0]} # https://unix.stackexchange.com/a/354604
        #QRCODE=$(head -n 1 "$ZBAR_STDOUT")
        
        # just assume it's a valid ToxID
        if [[ ! $QRCODE = "" ]]
        then
            # return result via qrcode parameter
            # great potential for funny qrcode exploits ;D
            # source:
            # https://stackoverflow.com/a/3243034

            eval "$1='$QRCODE'" # TODO: please someone escape this sh*t correctly
            reason=scan
            break
        fi

        # check if zbarcam is still running
        if kill -s 0 $zbar_pid
        then
            :
        else 
            #echo exit due to zbar error
            cat $ZBAR_STDERR
            reason=subprocess_exit
            break
        fi
    done

    if [ $reason = subprocess_error ]; then
        # this might happen in display mode, when pressing 'q'
        # or on actual erros. try to return error code in this case
        cat $ZBAR_STDERR 1>&2
        wait $zbar_pid 2>/dev/null
        ret=$?
    else
        kill $zbar_pid
        wait $zbar_pid 2>/dev/null
    fi
    rm -f $ZBAR_STDOUT $ZBAR_STDERR
    pkill zbarcam

    [ $reason = key ] && ret=0

    return $ret
}
