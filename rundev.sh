#!/bin/sh
ip=`hostname -I | tr -d ' '`
cat index.html | sed 's/\/ws:80/:7683/g' | sed "s/pappaochson\.se/$ip/g" > /website/www/tst/gamedev.html
cp *.png /website/www/tst
echo "http://$ip/tst/gamedev.html"
cp lws-minimal-ws-server jcserv
cp joystick.js /website/www/tst
./jcserv
