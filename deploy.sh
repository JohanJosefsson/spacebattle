#!/bin/sh
sudo systemctl stop jcserv.service
cp game2.html /website/www/game2.html
cp joystick.js /website/www/
cp *.png /website/www/
cp lws-minimal-ws-server /website/jcserv/jcserv
sudo systemctl start jcserv.service
