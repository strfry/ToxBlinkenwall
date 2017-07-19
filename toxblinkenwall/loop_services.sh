#! /bin/bash

function clean_up {

	# Perform program exit cleanup of framebuffer
	scripts/stop_loading_endless.sh
	scripts/cleanup_fb.sh
	exit
}

cd $(dirname "$0")
export LD_LIBRARY_PATH=usr/lib

# ---- only for RASPI ----
sudo /etc/init.d/lightdm start
sleep 4
# ---- only for RASPI ----

trap clean_up SIGHUP SIGINT SIGTERM

chmod u+x scripts/*.sh
chmod u+x toxblinkenwall
scripts/stop_loading_endless.sh
scripts/init.sh
sleep 2
scripts/create_gfx.sh

while [ 1 == 1 ]; do
	#####################################################
	# pick first available video device
	# change for your needs here!
	video_device=$(ls -1 /dev/video*|tail -1)
	#
	#####################################################

	scripts/stop_loading_endless.sh
	scripts/init.sh
	./toxblinkenwall -d "$video_device"
	sleep 10
done

