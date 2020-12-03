-- MySQL dump 10.10
--
-- Host: localhost    Database: current
-- ------------------------------------------------------
-- Server version	5.0.24a-Debian_9-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `credentials`
--

DROP TABLE IF EXISTS `credentials`;
CREATE TABLE `credentials` (
  `id` int(10) unsigned NOT NULL,
  `username` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `enabled` tinyint(1) default '1',
  `status_ts` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`username`),
  UNIQUE KEY `id` (`id`),
  UNIQUE KEY `username` (`username`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `credentials`
--


/*!40000 ALTER TABLE `credentials` DISABLE KEYS */;
LOCK TABLES `credentials` WRITE;
INSERT INTO `credentials` VALUES (1,'acidfoo','foo',1,'2007-02-02 01:39:53'),(2,'syncros','sync0xpwd',1,'2007-02-02 02:53:35'),(3,'jix','jix0xpwd',1,'2007-02-02 02:54:03'),(4,'subnet120','subnet120',0,'2007-02-02 02:55:11'),(5,'subnet124','subnet124',0,'2007-02-02 02:55:20'),(6,'guest001','guest001',0,'2007-02-02 02:56:27'),(7,'guest002','guest002',0,'2007-02-02 02:56:33'),(8,'guest003','guest003',0,'2007-02-02 02:56:40'),(9,'guest004','guest004',0,'2007-02-02 02:56:47'),(10,'guest005','guest005',0,'2007-02-02 02:56:54'),(11,'guest006','guest006',0,'2007-02-02 02:57:04'),(12,'guest007','guest007',0,'2007-02-02 02:57:10'),(13,'guest008','guest008',0,'2007-02-02 02:57:17'),(14,'guest009','guest009',0,'2007-02-02 02:57:26'),(15,'guest010','guest010',0,'2007-02-02 02:57:34');
UNLOCK TABLES;
/*!40000 ALTER TABLE `credentials` ENABLE KEYS */;

--
-- Table structure for table `groups_services`
--

DROP TABLE IF EXISTS `groups_services`;
CREATE TABLE `groups_services` (
  `name` varchar(32) NOT NULL,
  `description` blob,
  `route` varchar(128) default NULL,
  `controller` varchar(128) default NULL,
  PRIMARY KEY  (`name`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `groups_services`
--


/*!40000 ALTER TABLE `groups_services` DISABLE KEYS */;
LOCK TABLES `groups_services` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `groups_services` ENABLE KEYS */;

--
-- Table structure for table `groups_services_map`
--

DROP TABLE IF EXISTS `groups_services_map`;
CREATE TABLE `groups_services_map` (
  `group` varchar(32) NOT NULL,
  `name` varchar(32) NOT NULL,
  `enabled` tinyint(1) default '1',
  `status_ts` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`group`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `groups_services_map`
--


/*!40000 ALTER TABLE `groups_services_map` DISABLE KEYS */;
LOCK TABLES `groups_services_map` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `groups_services_map` ENABLE KEYS */;

--
-- Table structure for table `statistics`
--

DROP TABLE IF EXISTS `statistics`;
CREATE TABLE `statistics` (
  `id` int(10) unsigned NOT NULL,
  `duration` int(10) unsigned NOT NULL,
  `bytes_in` bigint(20) unsigned NOT NULL,
  `bytes_out` bigint(20) unsigned NOT NULL,
  `pckts_in` bigint(20) unsigned NOT NULL,
  `pckts_out` bigint(20) unsigned NOT NULL,
  `remote_ip` int(10) unsigned default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`,`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `statistics`
--


/*!40000 ALTER TABLE `statistics` DISABLE KEYS */;
LOCK TABLES `statistics` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `statistics` ENABLE KEYS */;

--
-- Table structure for table `subnets`
--

DROP TABLE IF EXISTS `subnets`;
CREATE TABLE `subnets` (
  `id` int(10) unsigned NOT NULL default '0',
  `subnet` varchar(15) NOT NULL,
  `netmask` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`id`,`subnet`),
  UNIQUE KEY `subnet` (`subnet`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `subnets`
--


/*!40000 ALTER TABLE `subnets` DISABLE KEYS */;
LOCK TABLES `subnets` WRITE;
INSERT INTO `subnets` VALUES (1,'72.55.145.97',32),(2,'72.55.145.98',32),(3,'72.55.145.99',32),(6,'72.55.145.100',32),(7,'72.55.145.101',32),(8,'72.55.145.102',32),(9,'72.55.145.103',32),(10,'72.55.145.104',32),(11,'72.55.145.105',32),(12,'72.55.145.106',32),(13,'72.55.145.107',32),(14,'72.55.145.108',32),(15,'72.55.145.109',32),(0,'72.55.145.110',32),(0,'72.55.145.111',32),(0,'72.55.145.112',32),(0,'72.55.145.113',32),(0,'72.55.145.114',32),(0,'72.55.145.115',32),(0,'72.55.145.116',32),(0,'72.55.145.117',32),(4,'72.55.145.118',32),(5,'72.55.145.119',32),(4,'72.55.145.120',30),(5,'72.55.145.124',30);
UNLOCK TABLES;
/*!40000 ALTER TABLE `subnets` ENABLE KEYS */;

--
-- Table structure for table `users_details`
--

DROP TABLE IF EXISTS `users_details`;
CREATE TABLE `users_details` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `ref_id` int(10) unsigned NOT NULL default '0',
  `first_name` varchar(32) NOT NULL,
  `last_name` varchar(32) NOT NULL,
  `email` varchar(128) NOT NULL,
  `company` varchar(64) default NULL,
  `telephone` varchar(16) default NULL,
  `cellphone` varchar(16) default NULL,
  `since` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`,`ref_id`),
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `users_details`
--


/*!40000 ALTER TABLE `users_details` DISABLE KEYS */;
LOCK TABLES `users_details` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `users_details` ENABLE KEYS */;

--
-- Table structure for table `users_services`
--

DROP TABLE IF EXISTS `users_services`;
CREATE TABLE `users_services` (
  `name` varchar(32) NOT NULL,
  `description` blob,
  `route` varchar(128) default NULL,
  `controller` varchar(128) default NULL,
  PRIMARY KEY  (`name`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `users_services`
--


/*!40000 ALTER TABLE `users_services` DISABLE KEYS */;
LOCK TABLES `users_services` WRITE;
INSERT INTO `users_services` VALUES ('pipeline',NULL,NULL,NULL);
UNLOCK TABLES;
/*!40000 ALTER TABLE `users_services` ENABLE KEYS */;

--
-- Table structure for table `users_services_map`
--

DROP TABLE IF EXISTS `users_services_map`;
CREATE TABLE `users_services_map` (
  `id` int(10) unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `enabled` tinyint(1) default '1',
  `status_ts` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`,`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `users_services_map`
--


/*!40000 ALTER TABLE `users_services_map` DISABLE KEYS */;
LOCK TABLES `users_services_map` WRITE;
INSERT INTO `users_services_map` VALUES (1,'pipeline',1,'2007-02-02 02:04:34'),(2,'pipeline',1,'2007-02-02 03:00:26'),(3,'pipeline',1,'2007-02-02 03:00:30'),(4,'pipeline',1,'2007-02-02 03:00:36'),(5,'pipeline',1,'2007-02-02 03:00:39'),(6,'pipeline',1,'2007-02-02 03:00:42'),(7,'pipeline',1,'2007-02-02 03:00:44'),(8,'pipeline',1,'2007-02-02 03:00:47'),(9,'pipeline',1,'2007-02-02 03:00:50'),(10,'pipeline',1,'2007-02-02 03:00:54'),(11,'pipeline',1,'2007-02-02 03:00:56'),(12,'pipeline',1,'2007-02-02 03:01:02'),(13,'pipeline',1,'2007-02-02 03:01:05'),(14,'pipeline',1,'2007-02-02 03:01:08'),(15,'pipeline',1,'2007-02-02 03:01:10');
UNLOCK TABLES;
/*!40000 ALTER TABLE `users_services_map` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

