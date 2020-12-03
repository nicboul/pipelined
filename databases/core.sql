-- This file creates the core services database.
-- Remember: each company needs its own database!
--
-- CONFIG_DB_NAME       Name of database (company name)

-- CONFIG_USER_ADMIN    Generic username for administrative website
-- CONFIG_PASS_ADMIN    Generic password for administrative website
-- CONFIG_HOST_ADMIN    Host running this Web server

-- CONFIG_USER_WEB      Generic username for public website
-- CONFIG_PASS_WEB      Generic password for public website
-- CONFIG_HOST_WEB      Host running this Web server

-- CONFIG_USER_PPP      Username used by PPP daemon
-- CONFIG_PASS_PPP      Password used by PPP daemon
-- CONFIG_HOST_PPP      Host running this PPP daemon

USE `mysql`;

--
-- We declare all services username and password here
--
REPLACE INTO `user` (`host`, `user`, `password`)
    VALUES (
        'CONFIG_HOST_ADMIN',
        'CONFIG_USER_ADMIN',
        PASSWORD('CONFIG_PASS_ADMIN')
);

REPLACE INTO `user` (`host`, `user`, `password`)
    VALUES (
        'CONFIG_HOST_WEB',
        'CONFIG_USER_WEB',
        PASSWORD('CONFIG_PASS_WEB')
);

REPLACE INTO `user` (`host`, `user`, `password`)
    VALUES (
        'CONFIG_HOST_PPP',
        'CONFIG_USER_PPP',
        PASSWORD('CONFIG_PASS_PPP')
);

--
-- Administrator should be able to modify anything but not the scheme.
--
REPLACE INTO `db` (`host`, `db`, `user`,
                   `select_priv`, `insert_priv`, `update_priv`, `delete_priv`)
    VALUES (
        'CONFIG_HOST_ADMIN',
        'CONFIG_DB_NAME',
        'CONFIG_USER_ADMIN',
        'Y', 'Y', 'Y', 'Y'
);

FLUSH PRIVILEGES;

DROP DATABASE IF EXISTS `CONFIG_DB_NAME`;
CREATE DATABASE `CONFIG_DB_NAME`;

USE `CONFIG_DB_NAME`;

-- credentials: description
--
-- @id          INT   Unique identifier for a login
-- @username    STR   Customer's login name
-- @password    STR   Customer's login password
-- @enabled     BOOL  Status of this account -- enabled or not
-- @status_ts   TIME  Timestamp of the status
--
CREATE TABLE `credentials` (
    `id`        INTEGER UNSIGNED NOT NULL UNIQUE,
    `username`  VARCHAR(255) NOT NULL UNIQUE,
    `password`  VARCHAR(255) NOT NULL,
    `enabled`   BOOLEAN DEFAULT TRUE,
    `status_ts` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`username`)
);

-- credentials: privileges
--
--  READ ONLY: pppd
--  READ/WRITE: web
--
GRANT SELECT ON `credentials` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';
GRANT SELECT, UPDATE ON `credentials` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';

-- users_details: description
--
-- @id           INT   Unique identifier for a login
-- @ref_id       INT   A link to the billing account
-- @first_name   STR   Customer's first name
-- @last_name    STR   Customer's last name
-- @company      STR   Customer's company name
-- @telephone    STR   Customer's telephone number
-- @cellphone    STR   Customer's cellphone number
-- @since        TIME  Timestamp of initial registration
--
CREATE TABLE `users_details` (
    `id`         INTEGER UNSIGNED NOT NULL UNIQUE AUTO_INCREMENT,
    `ref_id`     INTEGER UNSIGNED NOT NULL DEFAULT 0,
    `first_name` VARCHAR(32) NOT NULL,
    `last_name`  VARCHAR(32) NOT NULL,
    `email`      VARCHAR(128) NOT NULL,
    `company`    VARCHAR(64),
    `telephone`  VARCHAR(16),
    `cellphone`  VARCHAR(16),
    `since`      TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    PRIMARY KEY (`id`, `ref_id`)
);

-- users_details: privileges
--
--  READ ONLY:
--  READ/WRITE: web
--
GRANT SELECT, UPDATE ON `users_details` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';

-- users_services: description
--
-- @name            STR       Name of this service. MUST be unique
-- @description     STR       Description of this service
-- @route           STR       The URI that maps to the controller
-- @controller      STR       Web module to load for this service
--
CREATE TABLE `users_services` (
    `name`          VARCHAR(32) NOT NULL UNIQUE,
    `description`   BLOB,
    `route`         VARCHAR(128),
    `controller`    VARCHAR(128),

    PRIMARY KEY (`name`)
);

-- users_services: privileges
--
--  READ ONLY: web, pppd
--  READ/WRITE:
--
GRANT SELECT ON `users_services` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT SELECT ON `users_services` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- users_services_map: description
--
-- @id          INT   Unique identifier for a login
-- @name        STR   Name of the service
-- @enabled     BOOL  Is that service enabled ?
-- @status_ts   TIME  Timestamp of the status
--
CREATE TABLE `users_services_map` (
    `id`        INTEGER UNSIGNED NOT NULL,
    `name`      VARCHAR(32) NOT NULL,
    `enabled`   BOOLEAN DEFAULT TRUE,
    `status_ts` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`id`, `name`)
);

-- users_services_map: privileges
--
--  READ ONLY: pppd
--  READ/WRITE: web
--
GRANT SELECT, UPDATE, INSERT
             ON `users_services_map` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT SELECT ON `users_services_map` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- groups_services: description
--
-- @name            STR       Name of this service. MUST be unique
-- @description     STR       Description of this service
-- @route           STR       The URI that maps to the controller
-- @controller      STR       Web module to load for this service
--
CREATE TABLE `groups_services` (
    `name`          VARCHAR(32) NOT NULL UNIQUE,
    `description`   BLOB,
    `route`         VARCHAR(128),
    `controller`    VARCHAR(128),

    PRIMARY KEY (`name`)
);

-- groups_services: privileges
--
--  READ ONLY: web, pppd
--  READ/WRITE:
--
GRANT SELECT ON `groups_services` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT SELECT ON `groups_services` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- groups_services_map: description
--
-- @group       STR     Name of the group
-- @name        STR     Name of the service
-- @enabled     BOOL    Is that service enabled ?
-- @status_ts   TIME    Timestamp of the status
--
CREATE TABLE `groups_services_map` (
    `group`     VARCHAR(32) NOT NULL,
    `name`      VARCHAR(32) NOT NULL,
    `enabled`   BOOLEAN DEFAULT TRUE,
    `status_ts` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`group`, `name`)
);

-- groups_services_map: privileges
--
--  READ ONLY: pppd
--  READ/WRITE: web
--
GRANT SELECT, UPDATE, INSERT
             ON `groups_services_map` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT SELECT ON `groups_services_map` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- subnets: description
--
-- @id          INT   Unique identifier for a login
--                    Null only if subnet not used (yet)
-- @subnet      STR   Quad-dotted IP Address notation
-- @netmask     INT   CIDR notation network mask
--                    Obviously, endpoint addresses can be found
--                    using netmask = 32
--
CREATE TABLE `subnets` (
    `id`        INTEGER UNSIGNED,
    `subnet`    VARCHAR(15) NOT NULL UNIQUE,
    `netmask`   TINYINT UNSIGNED NOT NULL,

    PRIMARY KEY (`id`, `subnet`)
);

-- subnets: privileges
--
--  READ ONLY: web, pppd
--  READ/WRITE:
--
GRANT SELECT ON `subnets` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT SELECT ON `subnets` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- statistics: description
--
-- NOTE: The in and out statistics are switched to reflect
--       the other side of the tunnel.
--
-- @id          INT   Unique identifier for a login
-- @duration    INT   Connection duration time
-- @bytes_in    INT   Number of bytes sent to client
-- @bytes_out   INT   Number of bytes received from client
-- @pckts_in    INT   Number of packets sent to client
-- @pckts_out   INT   Number of packets received from client
-- @remote_ip   INT   Client IP where the connection came from
-- @timestamp   TIME  Current timestamp when adding the new row
--
CREATE TABLE `statistics` (
    `id`        INTEGER UNSIGNED NOT NULL,
    `duration`  INTEGER UNSIGNED NOT NULL,
    `bytes_in`  BIGINT  UNSIGNED NOT NULL,
    `bytes_out` BIGINT  UNSIGNED NOT NULL,
    `pckts_in`  BIGINT  UNSIGNED NOT NULL,
    `pckts_out` BIGINT  UNSIGNED NOT NULL,
    `remote_ip` INTEGER UNSIGNED,
    `timestamp` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (`id`, `timestamp`)
);

-- statistics: privileges
--
--  READ ONLY: web
--  READ/WRITE: pppd
--
GRANT SELECT ON `statistics` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT INSERT ON `statistics` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- tunnel_throttling: description
--
-- @id          STR     Unique identifier for a login
-- @rate        INT     Maximum rate in kilobits
--
CREATE TABLE `tunnel_throttling` (
    `id`        INTEGER UNSIGNED NOT NULL,
    `rate`      INTEGER UNSIGNED NOT NULL,

    PRIMARY KEY (`id`)
);

-- tunnel_throttling: privileges
--
--  READ ONLY: pppd
--  READ/WRITE: web
--
GRANT INSERT ON `tunnel_throttling` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT SELECT ON `tunnel_throttling` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';

-- tunnel_status: description
--
-- @id              STR     Unique identifier for a login
-- @tun_device      STR     Name of the tunnel device
-- @remote_ip       STR     Quad dotted real client IPv4 address
-- @is_online       BOOL    True if the tunnel is online
-- @timestamp       TIME    Timestamp of last status update
--
CREATE TABLE `tunnel_status` (
    `id`            INTEGER UNSIGNED NOT NULL,
    `tun_device`    VARCHAR(16) NOT NULL,
    `remote_ip`     VARCHAR(15) NOT NULL,
    `is_online`     BOOLEAN DEFAULT TRUE,
    `timestamp`     TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- tunnel_status: privileges
--
--  READ ONLY: web
--  READ/WRITE: pppd
--
GRANT SELECT ON `tunnel_status` TO 'CONFIG_USER_WEB'@'CONFIG_HOST_WEB';
GRANT INSERT,
      UPDATE ON `tunnel_status` TO 'CONFIG_USER_PPP'@'CONFIG_HOST_PPP';
