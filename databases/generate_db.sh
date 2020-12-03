#!/bin/sh
#
# This scripts imports and populates the core database into MySQL
#

DB_HOST=localhost
DB_USERNAME=root

CONFIG_DB_NAME=xiad

CONFIG_USER_ADMIN=${CONFIG_DB_NAME}_admin
CONFIG_PASS_ADMIN=${CONFIG_DB_NAME}_admin0xpass
CONFIG_HOST_ADMIN=localhost

CONFIG_USER_WEB=${CONFIG_DB_NAME}_www
CONFIG_PASS_WEB=${CONFIG_DB_NAME}_www0xpass
CONFIG_HOST_WEB=localhost

CONFIG_USER_PPP=${CONFIG_DB_NAME}_ppp
CONFIG_PASS_PPP=${CONFIG_DB_NAME}_ppp0xpass
CONFIG_HOST_PPP=%

# This is a variable substitution kludge!

cat core.sql  | sed s/CONFIG_DB_NAME/$CONFIG_DB_NAME/g                    \
            | sed s/CONFIG_USER_ADMIN/$CONFIG_USER_ADMIN/g              \
            | sed s/CONFIG_PASS_ADMIN/$CONFIG_PASS_ADMIN/g              \
            | sed s/CONFIG_HOST_ADMIN/$CONFIG_HOST_ADMIN/g              \
            | sed s/CONFIG_USER_WEB/$CONFIG_USER_WEB/g                  \
            | sed s/CONFIG_PASS_WEB/$CONFIG_PASS_WEB/g                  \
            | sed s/CONFIG_HOST_WEB/$CONFIG_HOST_WEB/g                  \
            | sed s/CONFIG_USER_PPP/$CONFIG_USER_PPP/g                  \
            | sed s/CONFIG_PASS_PPP/$CONFIG_PASS_PPP/g                  \
            | sed s/CONFIG_HOST_PPP/$CONFIG_HOST_PPP/g                  \
            | mysql -v -v -v -h $DB_HOST -u $DB_USERNAME -p
