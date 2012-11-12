#!/bin/bash
# Author : Abhiram R

cd bots/AlkorBot/
make clean
make
cd ../v6/
make clean
make
cd ../v5/
make clean
make
cd ../v4/
make clean
make
cd ../../
echo "Fighting Rajan as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/GodiBot/rajan_2.so
echo "Fighting Rajan as Red ..."
bin/Desdemona bots/GodiBot/rajan_2.so bots/AlkorBot/bot.so
echo "Fighting Mouli as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/GodiBot/bot_ch.so
echo "Fighting Mouli as Red ..."
bin/Desdemona bots/GodiBot/bot_ch.so bots/AlkorBot/bot.so
echo "Fighting bapat latest as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/GodiBot/bot_bapat.so
echo "Fighting bapat latest as Red ..."
bin/Desdemona bots/GodiBot/bot_bapat.so bots/AlkorBot/bot.so

echo "Fighting v6 as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/v6/bot.so
echo "Fighting v6 as Red ..."
bin/Desdemona bots/v6/bot.so bots/AlkorBot/bot.so

echo "Fighting v5 as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/v5/bot.so
echo "Fighting v5 as Red ..."
bin/Desdemona bots/v5/bot.so bots/AlkorBot/bot.so

echo "Fighting v4 as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/v4/bot.so
echo "Fighting v4 as Red ..."
bin/Desdemona bots/v4/bot.so bots/AlkorBot/bot.so

echo "Fighting Bapat as Black ..."
bin/Desdemona bots/AlkorBot/bot.so bots/GodiBot/bapat.so
echo "Fighting Bapat as Red ..."
bin/Desdemona bots/GodiBot/bapat.so bots/AlkorBot/bot.so





