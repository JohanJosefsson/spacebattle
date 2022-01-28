#!/bin/sh
ip=`hostname -I | tr -d ' '`
cat game2.html | sed 's/\/ws:80/:7683/g' | sed "s/pappaochson\.se/$ip/g" > /website/www/gamedev.html
cp *.png /website/www/
echo "http://$ip/gamedev.html"
cp lws-minimal-ws-server jcserv
cp joystick.js /website/www/
./jcserv
