#!/bin/sh
sudo systemctl stop jcserv.service
cp game.html /website/www/spacebattle
cp joystick.js /website/www/spacebattle
cp *.png /website/www/spacebattle
cp lws-minimal-ws-server /website/jcserv/jcserv
sudo systemctl start jcserv.service
