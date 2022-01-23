#!/bin/sh
ip=`hostname -I | tr -d ' '`
cat game2.html | sed 's/:7682/:7683/g' | sed "s/pappaochson\.se/$ip/g" > /website/www/gamedev.html
cp spaceship.png /website/www/
echo "http://$ip/gamedev.html"
cp lws-minimal-ws-server jcserv
./jcserv