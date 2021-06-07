#!/bin/sh

case "$1" in
    -h|--help|?)
        echo "usage: ./import_rules.sh <the file of rules>"
        exit 0
        ;;
esac

if [ ! -n "$1" ]; then
    echo "usage: ./import_rules.sh <the file of rules>"
    exit
fi

# database info----------
db_user='fw'
db_pswd='scaufw'
db_name="firewall"
table_name="rules"
# -----------------------

temp=$(mktemp tmp.XXXXXX)
cat $1 > $temp

# 转义，将路径里一个反斜杠=>六个反斜杠
sed -i 's/\\/\\\\\\\\\\\\/g' $temp

echo "Item list:"
while read line
do
    case $((i++ % 2)) in
        0) pg_name=$(echo -e $line);;
        1) pg_path=$(echo -e $line);echo -e $pg_name;echo -e $pg_path;
            mysql -u$db_user -p$db_pswd -e"insert ignore into $db_name.$table_name(pg_name, pg_path) values('$pg_name','$pg_path');";;
    esac
done < $temp

rm $temp
